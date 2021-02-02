#include <fstream>
#include "src/ads_feature.h"

int main(int argc, char** argv) {
    std::unordered_map<std::string, int32_t> fs;
    std::ifstream file(argv[1]);
    Feature feature;
    if (file.is_open()) {
        std::string line;
        while (std::getline(file, line)) {
            ModelFeature mf;
            auto ret = mf.extract_json(line);
            printf("%s\n", line.c_str());
            printf("%s\n", ret.c_str());
            mf.print();
        }
        file.close();
    }

    return 0;
}
