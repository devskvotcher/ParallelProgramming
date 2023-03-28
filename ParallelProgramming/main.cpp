#include <iostream>
#include <fstream>
#include <string>
#include <vector>
#include <omp.h>
#include <mpi.h>

bool search_substring(const std::string& filename, const std::string& substring) {
    std::ifstream file(filename);
    std::string line;
    while (std::getline(file, line)) {
        if (line.find(substring) != std::string::npos) {
            return true;
        }
    }
    return false;
}
std::vector<std::string> CreateFiles(std::vector<std::string> &filenames)
{
    size_t number_files;
    do
    {
        std::cout << "How many files do you want to create?";
        std::cin >> number_files;
        number_files = static_cast<size_t>(number_files);
    } while (number_files < 10 || number_files > 20);    
    for (size_t i = 0; i < number_files; i++)
    {
        std::string filename = "file" + std::to_string(i) + ".txt";
        filenames.push_back(filename);
       // PutFiles(filename);
    }
    return filenames;
}
void GenerateData(unsigned int &sizeOfFile, std::string filename)
{
    int minWordLength = 2, maxWordLength = 30;
    std::ofstream fout(filename, std::ios_base::out);
    //определ€ем длину первого слова
    int currentWordLength = minWordLength + rand() % (maxWordLength - minWordLength);
    int symbolsInFile = 0;
    while (symbolsInFile < sizeOfFile)
    {
        if (!currentWordLength)
        {
            //≈сли завершили формировани€ слова, то печатаем пробел и
            fout << " ";
            ++symbolsInFile;
            //определ€ем длину следующего слова
            currentWordLength = minWordLength + rand() % (maxWordLength - minWordLength);
        }
        //ƒобавл€ем в текущее слово новый символ
        fout << (char)((rand() % ('z' - 'a' + 1)) + 'a');
        --currentWordLength;
        ++symbolsInFile;
    }
    fout.close();
}
void PutFiles(std::vector<std::string>& filenames)
{
    
    for (size_t i = 0; i < filenames.size(); i++)
    {        
        unsigned int sizeOfFiles = rand() % 21 + 10;
        sizeOfFiles = sizeOfFiles * 1024 * 1024 / sizeof(char);
        GenerateData(sizeOfFiles, filenames.at(i));
        std::cout << "‘айл " << filenames.at(i) << " успешно создан" << std::endl;
    }
    
}
int main(int argc, char* argv[]) 
{
    srand(time(nullptr));
    // Initialize MPI
    MPI_Init(&argc, &argv);

    int world_size, world_rank;
    MPI_Comm_size(MPI_COMM_WORLD, &world_size);
    MPI_Comm_rank(MPI_COMM_WORLD, &world_rank);
    //--------------------------
    size_t number_files ;
    std::vector<std::string> filenames;
    CreateFiles(filenames);
    PutFiles(filenames);
    std::string substring;
    std::cout << "What substring are you looking for?";
    std::cin >> substring;
    int num_files = filenames.size();

    // Sequential version
    if (world_rank == 0) {
        std::cout << "Sequential version:" << std::endl;
        for (const auto& filename : filenames) {
            if (search_substring(filename, substring)) {
                std::cout << "Substring found in " << filename << std::endl;
            }
            else {
                std::cout << "Substring not found in " << filename << std::endl;
            }
        }
    }
    // OpenMP version
#pragma omp parallel for if (world_rank == 0)
    for (int i = 0; i < num_files; i++) {
        if (search_substring(filenames[i], substring)) {
#pragma omp critical
            std::cout << "OpenMP: Substring found in " << filenames[i] << std::endl;
        }
        else {
#pragma omp critical
            std::cout << "OpenMP: Substring not found in " << filenames[i] << std::endl;
        }
    }

    // MPI version
    int files_per_process = num_files / world_size;
    for (int i = world_rank * files_per_process; i < (world_rank + 1) * files_per_process; i++) {
        if (i < num_files) {
            if (search_substring(filenames[i], substring)) {
                std::cout << "MPI: Substring found in " << filenames[i] << " by process " << world_rank << std::endl;
            }
            else {
                std::cout << "MPI: Substring not found in " << filenames[i] << " by process " << world_rank << std::endl;
            }
        }
    }

    // Finalize MPI
    MPI_Finalize();
    return 0;
}