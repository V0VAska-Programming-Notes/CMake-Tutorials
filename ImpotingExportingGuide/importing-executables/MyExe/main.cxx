// A simple program that outputs a file with the given name
#include <fstream>
#include <iostream>

int main(int argc, char* argv[])
{
    std::ofstream outfile("main.cpp");
    outfile << "#include <iostream>" << std::endl;
    outfile << "int main(int argc, char* argv[])" << std::endl;
    outfile << "{" << std::endl;
    outfile << "    // Your code here" << std::endl;
    outfile << "    std::cout << \"Hello, World!!!\" << std::endl;" << std::endl;
    outfile << "    return 0;" << std::endl;
    outfile << "}" << std::endl << std::endl;
    outfile.close();

    return 0;
}
