#include <iostream>
#include <elf.h>
#include <libelf.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h> // for close()
#include <cstring> // for strerror()
#include <errno.h> // for errno

int main(int argc, char *argv[]) {
    if (argc < 2) {
        std::cerr << "Usage: " << argv[0] << " <path_to_elf_file>" << std::endl;
        return 1;
    }

    const char* elf_path = argv[1];

    // Open the ELF file
    int fd = open(elf_path, O_RDONLY);
    if (fd == -1) {
        std::cerr << "Failed to open the ELF file: " << strerror(errno) << std::endl;
        return 1;
    }

    // Initialize the ELF library
    if (elf_version(EV_CURRENT) == EV_NONE) {
        std::cerr << "Failed to initialize the ELF library: " << elf_errmsg(-1) << std::endl;
        close(fd);
        return 1;
    }

    Elf *elf = elf_begin(fd, ELF_C_READ, NULL);
    if (elf == NULL) {
        std::cerr << "Failed to initialize the ELF library: " << elf_errmsg(-1) << std::endl;
        close(fd);
        return 1;
    }

    // Get the ELF header
    Elf64_Ehdr *ehdr = elf64_getehdr(elf);
    if (ehdr == NULL) {
        std::cerr << "Failed to get the ELF header: " << elf_errmsg(-1) << std::endl;
        elf_end(elf);
        close(fd);
        return 1;
    }

    // Iterate through the sections
    Elf_Scn *scn = NULL;
    while ((scn = elf_nextscn(elf, scn)) != NULL) {
        Elf64_Shdr *shdr = elf64_getshdr(scn);
        if (shdr == NULL) {
            std::cerr << "Failed to get the section header: " << elf_errmsg(-1) << std::endl;
            elf_end(elf);
            close(fd);
            return 1;
        }

        // Check if the section contains executable code
        if (shdr->sh_flags & SHF_EXECINSTR) {
            // Get the section data
            Elf_Data *data = elf_getdata(scn, NULL);
            if (data == NULL) {
                std::cerr << "Failed to get the section data: " << elf_errmsg(-1) << std::endl;
                elf_end(elf);
                close(fd);
                return 1;
            }

            // Disassemble the code
            for (size_t i = 0; i < shdr->sh_size; ++i) {
                // Print the instruction address and byte
                std::cout << std::hex << (shdr->sh_addr + i) << ": " 
                          << std::hex << static_cast<int>(static_cast<unsigned char*>(data->d_buf)[i]) 
                          << std::endl;

                // TODO: Implement instruction disassembly using libdwarf or other libraries
            }
        }
    }

    // Clean up
    elf_end(elf);
    close(fd);

    std::cout << "Disassembly finished." << std::endl;

    return 0;
}
