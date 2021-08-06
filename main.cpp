#include <iostream>
#include <fstream>
#include <sstream>
#include <vector>
#include <sys/stat.h>
#include "ImgSpec.h"

#define CHUNK_SIZE 32
#define QUIT 1

bool IsPathExist(const std::string& s)
{
    struct stat buffer;
    return (stat(s.c_str(), &buffer) == 0);
}


void Unscramble(int numRows, unsigned char* array, unsigned char* resolveArray);
std::string IntToString3Width(int value);
unsigned char FindInPalette(unsigned char pixel[3], unsigned char palette[256][3], short& paletteSize);
void loadGameFile8Bit(int numRows, int width, std::string fileName, char* array, unsigned char palette[256][3], int offset);
int GetBMPSize(std::string fileName);
void loadBMPFile(int numRows, int numChunks, std::string fileName, unsigned char palette[256][3], unsigned char* imgArray);
void WidenArray(unsigned char* imgArray, unsigned char* hexarray, int arraySize);
void convertArray(int numRows, int numChunks, unsigned char* unscrambleArray, char* hexArray, unsigned char* hmmArray, unsigned char* finalArray);
void revertArray(int numRows, int numChunks, unsigned char* unscrambleArray, char* hexArray, unsigned char* hmmArray, unsigned char* finalArray);
void revertArray(int numRows, int numChunks, unsigned char* unscrambleArray, unsigned char* hexArray, unsigned char* hmmArray, unsigned char* finalArray);
void writeGameFile24Bit(int numRows, int numChunks, std::string fileName, unsigned char palette[256][3], unsigned char* imgArray, int offset);
void writeGameFile16Bit(int numRows, int numChunks, std::string fileName, unsigned char palette[256][3], unsigned char* imgArray, int offset);
void writeImgFile(int numRows, int numChunks, unsigned char* bmpHeader, std::string filename, unsigned char palette[256][3], unsigned char* finalArray);
void writeImgRaw(int width, int height, unsigned char* bmpHeader, std::string filename, unsigned char palette[256][3], unsigned char* finalArray);
void writeRaw(int numRows, int numChunks, char* bmpHeader, std::string filename, unsigned char palette[256][3], unsigned char* hexArray);
void writePalette(char* bmpHeader, int fileNum, unsigned char palette[256][3]);
void Palette16Bit(unsigned char paletteRGB[256][3], std::string filename, int offset, int paletteOffset);
void Palette24Bit(unsigned char paletteRGB[256][3], std::string filename, int offset, int paletteOffset);
void Scramble(int numRows, unsigned char* toArray, unsigned char* fromArray);
void FindOffset(std::string filename, std::vector<ImgSpec>& headerInfo);
int SetDirectories(std::string& homeDir, std::string& gameDir);
int InitializeDirectories(std::string& homeDir, std::string& gameDir);
int InputTexture(std::string& inFilePath, std::string& outFilePath, std::string& inFileName, std::vector<ImgSpec>& headerInfo);
void OutputImages(std::string& inFilePath, std::string& outFilePath, std::string& inFileName, std::vector<ImgSpec>& headerInfo);
void GenHeaderInfo(std::string inFilePath, std::string inFileName, std::vector<ImgSpec>& headerInfo);




int main() {
    std::string modestr;
    char mode;
    std::vector<ImgSpec> headerInfo;
    std::string fileID;
    std::string homeDir;
    std::string gameDir;

    while (mode != 'q') {
        std::cout << "Enter a mode:\n    i: input an image to a game file\n    o: output a gamefile to an image folder\n    g: output all gallery images at once\n    c: configure directories\n    q: quit\n:";
        mode = '\0';
        std::getline(std::cin, modestr);
        mode = modestr.at(0);

        if (mode == 'c') {
            SetDirectories(homeDir, gameDir);
        }

        else if (mode == 'i' || mode == 'o' || mode == 'g') {
            InitializeDirectories(homeDir, gameDir);
        }

        else {
            continue;
        }


        if (mode == 'i') {
            
            std::cout << "WARNING: THIS TOOL MAY PERMANENTLY ALTER AND POTENTIALLY CORRUPT GAME FILES. It is highly recommended to create a backup of \n";
            std::cout << gameDir << " \nbefore continuing. Continue? (y/n): ";
            std::string tempString;
            std::getline(std::cin, tempString);
            if (tempString != "y") {
                std::cout << "Quitting" << std::endl;
                return 0;
            }
            std::cout << "Enter a game filename (case-sensitive): ";
            std::getline(std::cin, fileID);
            std::fstream testStream;
            testStream.open(gameDir + fileID);
            while (!testStream.is_open()) {
                std::cout << "The file could not be opened. Enter another filename, or q to quit: ";
                std::getline(std::cin, fileID);
                if (fileID == "q") {
                    std::cout << "Quitting" << std::endl;
                    return 0;
                }
                else {
                    testStream.open(gameDir + fileID);
                }
            }
            testStream.close();
            GenHeaderInfo(gameDir, fileID, headerInfo);
            InputTexture(gameDir, homeDir, fileID, headerInfo);
            std::cout << std::endl;
        }


        if (mode == 'o') {
            std::cout << "Enter a game filename (case-sensitive): ";
            std::getline(std::cin, fileID);
            std::fstream testStream;
            testStream.open(gameDir + fileID);
            while (!testStream.is_open()) {
                std::cout << "The file could not be opened. Enter another filename, or q to quit: ";
                std::getline(std::cin, fileID);
                if (fileID == "q") {
                    std::cout << "Quitting" << std::endl;
                    return 0;
                }
                else {
                    testStream.open(gameDir + fileID);
                }
            }
            testStream.close();
            GenHeaderInfo(gameDir, fileID, headerInfo);
            OutputImages(gameDir, homeDir, fileID, headerInfo);
            std::cout << std::endl;
        }

        if (mode == 'g') {
            std::ifstream testIStream;
            testIStream.open(gameDir + "GAMEUI_GALLERY_001.BIN");
            if (!testIStream.is_open()) {
                std::cout << "Error: the gallery files could not be opened. Try checking your game directory." << std::endl;
                testIStream.close();
                continue;
            }
            testIStream.close();
            std::string fileNameList[8] = {"A01", "A02", "A03", "A04", "B01", "B02", "B03", "B04"};
            for (int i = 1; i < 146; i++) {
                fileID = "GAMEUI_GALLERY_" + IntToString3Width(i) + ".BIN";
                GenHeaderInfo(gameDir, fileID, headerInfo);
                OutputImages(gameDir, homeDir, fileID, headerInfo);
                std::cout << std::endl;
            }
            for (int i = 0; i < 8; i++) {
                fileID = "GAMEUI_GALLERY_" + fileNameList[i] + ".BIN";
                GenHeaderInfo(gameDir, fileID, headerInfo);
                OutputImages(gameDir, homeDir, fileID, headerInfo);
                std::cout << std::endl;
            }
        }
    }

    return 0;
}


void Unscramble(int numRows, unsigned char* array, unsigned char* resolveArray) {
    int order[16] = { 0,4,8,12,1,5,9,13,2,6,10,14,3,7,11,15 };
    int indices[4] = { 0,0,0,0 };
    int indices2[4] = { 0,0,0,0 };
    for (int i = 0; i < (CHUNK_SIZE / 2); i++) {
        if (order[i] % 2 == 0) {
            indices[1] = order[i];
            indices[0] = order[i] + 17;
            indices[2] = order[i] + 33;
            indices[3] = order[i] + 48;
            indices2[0] = order[i];
            indices2[1] = order[i] + 17;
            indices2[3] = order[i] + 33;
            indices2[2] = order[i] + 48;
        }
        else {
            indices[1] = order[i];
            indices[0] = order[i] + 15;
            indices[2] = order[i] + 31;
            indices[3] = order[i] + 48;
            indices2[0] = order[i];
            indices2[1] = order[i] + 15;
            indices2[3] = order[i] + 31;
            indices2[2] = order[i] + 48;
        }
        for (int j = 0; j < numRows / 2; j += 2) {
            for (int k = 0; k < 4; k++) {
                if ((i / 4) % 2 == 0) {
                    *(resolveArray + i * numRows * 2 + j * 4 + k * 2) = *(array + indices2[k] * numRows / 2 + j);
                    *(resolveArray + i * numRows * 2 + j * 4 + k * 2 + 1) = *(array + indices2[k] * numRows / 2 + j + 1);
                }
                else {
                    *(resolveArray + i * numRows * 2 + j * 4 + k * 2) = *(array + indices[k] * numRows / 2 + j);
                    *(resolveArray + i * numRows * 2 + j * 4 + k * 2 + 1) = *(array + indices[k] * numRows / 2 + j + 1);
                }
            }
        }
    }
}

void Scramble(int numRows, unsigned char* toArray, unsigned char* fromArray) {
    int order[16] = { 0,4,8,12,1,5,9,13,2,6,10,14,3,7,11,15 };
    int indices[4] = { 0,0,0,0 };
    int indices2[4] = { 0,0,0,0 };
    for (int i = 0; i < (CHUNK_SIZE / 2); i++) {
        if (order[i] % 2 == 0) {
            indices[1] = order[i];
            indices[0] = order[i] + 17;
            indices[2] = order[i] + 33;
            indices[3] = order[i] + 48;
            indices2[0] = order[i];
            indices2[1] = order[i] + 17;
            indices2[3] = order[i] + 33;
            indices2[2] = order[i] + 48;
        }
        else {
            indices[1] = order[i];
            indices[0] = order[i] + 15;
            indices[2] = order[i] + 31;
            indices[3] = order[i] + 48;
            indices2[0] = order[i];
            indices2[1] = order[i] + 15;
            indices2[3] = order[i] + 31;
            indices2[2] = order[i] + 48;
        }
        for (int j = 0; j < numRows / 2; j += 2) {
            for (int k = 0; k < 4; k++) {
                if ((i / 4) % 2 == 0) {
                    *(toArray + indices2[k] * numRows / 2 + j) = *(fromArray + i * numRows * 2 + j * 4 + k * 2);
                    *(toArray + indices2[k] * numRows / 2 + j + 1) = *(fromArray + i * numRows * 2 + j * 4 + k * 2 + 1);
                }
                else {
                    *(toArray + indices[k] * numRows / 2 + j) = *(fromArray + i * numRows * 2 + j * 4 + k * 2);
                    *(toArray + indices[k] * numRows / 2 + j + 1) = *(fromArray + i * numRows * 2 + j * 4 + k * 2 + 1);
                }
            }
        }
    }
}

void WidenArray(unsigned char* imgArray, unsigned char* hexArray, int arraySize) {
    for (int i = 0; i < arraySize/2; i++) {
        *(hexArray + i * 2) = (*(imgArray + i) >> 4) & 0x0F;
        *(hexArray + i * 2 + 1) = *(imgArray + i) & 0x0F;
    }
}

void Palette16Bit(unsigned char paletteRGB[256][3], std::string filename, int offset, int paletteOffset) {
    std::ifstream bits;
    bits.open(filename, std::ios::binary);
    bits.seekg(offset + paletteOffset);
    char bitColor[2] = { '\0','\0' };
    unsigned short bitVal = 0;
    int counter = 0;
    while (counter < 256) {
        bits.read(bitColor, 2);
        bitVal = *((unsigned short*)bitColor);
        paletteRGB[counter][0] = (bitVal & 0x7c00) >> 7;
        paletteRGB[counter][1] = (bitVal & 0x03e0) >> 2;
        paletteRGB[counter][2] = (bitVal & 0x001f) << 3;
        counter++;
    }
}

void Palette24Bit(unsigned char paletteRGB[256][3], std::string filename, int offset, int paletteOffset) {
    std::ifstream bits;
    bits.open(filename, std::ios::binary);
    bits.seekg(offset + paletteOffset);
    char bitColor[4] = { '\0','\0' };
    int counter = 0;
    while (counter < 256) {
        bits.read(bitColor, 4);
        paletteRGB[counter][0] = bitColor[2];
        paletteRGB[counter][1] = bitColor[1];
        paletteRGB[counter][2] = bitColor[0];
        counter++;
    }
}

std::string IntToString3Width(int value) {
    std::stringstream stream;
    stream << value;
    std::string numStr;
    stream >> numStr;
    while (numStr.size() < 3) {
        numStr = "0" + numStr;
    }
    if (value == 1000) {
        return "nightTest";
    }
    return numStr;
}

int GetBMPSize(std::string fileName) {
    char discard[28];
    char widthData[4];
    char heightData[4];
    char pixel[3];
    short paletteSize = 0;
    std::ifstream imgFile;
    imgFile.open(fileName, std::ios::binary);
    imgFile.read(discard, 18);
    imgFile.read(widthData, 4);
    imgFile.read(heightData, 4);
    return (*(int*)(&widthData) * *(int*)(&heightData));
}


void loadBMPFile(int numRows, int numChunks, std::string fileName, unsigned char palette[256][3], unsigned char* imgArray) {
    char discard[28];
    char widthData[4];
    char heightData[4];
    char pixel[3];
    short paletteSize = 0;
    std::ifstream imgFile;
    imgFile.open(fileName, std::ios::binary);
    imgFile.read(discard, 18);
    imgFile.read(widthData, 4);
    imgFile.read(heightData, 4);
    imgFile.read(discard, 28);
    int paddingSize = 4 - ((numChunks * CHUNK_SIZE / 2) * 3) % 4;
    if (paddingSize == 4) {
        paddingSize = 0;
    }
    for (int i = 0; i < numRows * 2; i++) {
        for (int j = 0; j < numChunks * CHUNK_SIZE / 2; j++) {
            imgFile.read(pixel, 3);
            *(imgArray + i * numChunks * CHUNK_SIZE / 2 + j) = FindInPalette((unsigned char*)pixel, palette, paletteSize);
        }
        imgFile.read(discard, paddingSize);
    }
    for (int i = paletteSize; i < 256; i++) {
        palette[i][0] = 0;
        palette[i][1] = 0;
        palette[i][2] = 0;
    }

}

unsigned char FindInPalette(unsigned char pixel[3], unsigned char palette[256][3], short& paletteSize) {
    for (int i = 0; i < paletteSize; i++) {
        if (pixel[0] == palette[i][2] && pixel[1] == palette[i][1] && pixel[2] == palette[i][0]) {
            return i;
        }
    }
    if (paletteSize < 256) {
        palette[paletteSize][2] = pixel[0];
        palette[paletteSize][1] = pixel[1];
        palette[paletteSize][0] = pixel[2];
        paletteSize++;
        return paletteSize - 1;
    }
    else {
        return 0;
    }
}

void loadGameFile8Bit(int numRows, int width, std::string fileName, char* array, unsigned char palette[256][3], int offset) {
    std::ifstream imgFile;
    imgFile.open(fileName, std::ios::binary);
    char hex[1] = { 0 };
    unsigned char temp[3] = { 0,0,0 };
    int counter = 0;
    while (!imgFile.eof()) {
        imgFile.read(hex, 1);
        if (counter >= offset && counter < offset + (width * (numRows))) {
            *(array + counter - offset) = *hex;
        }
        else if (counter > offset + (width * (numRows))) {
            break;
        }
        counter++;
    }

    for (int i = 8; i < 247; i++) {  //change back to 247
        if (i % 16 == 0) {
            i += 24;
        }
        temp[0] = palette[i][0];
        temp[1] = palette[i][1];
        temp[2] = palette[i][2];
        palette[i][0] = palette[i + 8][0];
        palette[i][1] = palette[i + 8][1];
        palette[i][2] = palette[i + 8][2];
        palette[i + 8][0] = temp[0];
        palette[i + 8][1] = temp[1];
        palette[i + 8][2] = temp[2];
    }
    imgFile.close();
}


void convertArray(int numRows, int numChunks,
    unsigned char* unscrambleArray,
    char* hexArray,
    unsigned char* hmmArray,
    unsigned char* finalArray) {  //[numRows*2][numChunks*CHUNK_SIZE/2]
    for (int i = 0; i < numChunks; i++) {
        for (int j = 0; j < numRows; j += 2) {
            for (int k = 0; k < CHUNK_SIZE; k++) {
                if ((j / 2) % 2 == 0) {
                    *(unscrambleArray + k * numRows / 2 + j / 2) = *(hexArray + j * numChunks * CHUNK_SIZE + (i * CHUNK_SIZE) + k);
                    *(unscrambleArray + k * numRows / 2 + j / 2 + 1) = *(hexArray + (j + 1) * numChunks * CHUNK_SIZE + (i * CHUNK_SIZE) + k);
                }
                else {
                    *(unscrambleArray + (k + 32) * numRows / 2 + j / 2 - 1) = *(hexArray + j * numChunks * CHUNK_SIZE + (i * CHUNK_SIZE) + k);
                    *(unscrambleArray + (k + 32) * numRows / 2 + j / 2) = *(hexArray + (j + 1) * numChunks * CHUNK_SIZE + (i * CHUNK_SIZE) + k);
                }
            }
        }
        Unscramble(numRows, unscrambleArray, hmmArray);

        for (int j = 0; j < CHUNK_SIZE / 2; j++) {
            for (int k = 0; k < numRows * 2; k++) {
                *(finalArray + k * numChunks * CHUNK_SIZE / 2 + i * CHUNK_SIZE / 2 + j) = *(hmmArray + j * numRows * 2 + k);
            }
        }
    }
}

void revertArray(int numRows, int numChunks,
    unsigned char* unscrambleArray,
    char* hexArray,
    unsigned char* hmmArray,
    unsigned char* finalArray) {
    for (int i = 0; i < numChunks; i++) {
        for (int j = 0; j < numRows * 2; j += 1) {
            for (int k = 0; k < CHUNK_SIZE / 2; k++) {
                *(hmmArray + k * numRows * 2 + j) = *(finalArray + j * numChunks * CHUNK_SIZE / 2 + i * CHUNK_SIZE / 2 + k);
            }
        }
        Scramble(numRows, unscrambleArray, hmmArray);
        for (int j = 0; j < numRows; j += 2) {
            for (int k = 0; k < CHUNK_SIZE; k++) {
                if ((j / 2) % 2 == 0) {
                    *(hexArray + j * numChunks * CHUNK_SIZE + (i * CHUNK_SIZE) + k) = *(unscrambleArray + k * numRows / 2 + j / 2);
                    *(hexArray + (j + 1) * numChunks * CHUNK_SIZE + (i * CHUNK_SIZE) + k) = *(unscrambleArray + k * numRows / 2 + j / 2 + 1);
                }
                else {
                    *(hexArray + j * numChunks * CHUNK_SIZE + (i * CHUNK_SIZE) + k) = *(unscrambleArray + (k + 32) * numRows / 2 + j / 2 - 1);
                    *(hexArray + (j + 1) * numChunks * CHUNK_SIZE + (i * CHUNK_SIZE) + k) = *(unscrambleArray + (k + 32) * numRows / 2 + j / 2);
                }
            }
        }
    }
}

void revertArray(int numRows, int numChunks,
    unsigned char* unscrambleArray,
    unsigned char* hexArray,
    unsigned char* hmmArray,
    unsigned char* finalArray) {
    for (int i = 0; i < numChunks; i++) {
        for (int j = 0; j < numRows * 2; j += 1) {
            for (int k = 0; k < CHUNK_SIZE / 2; k++) {
                *(hmmArray + k * numRows * 2 + j) = *(finalArray + j * numChunks * CHUNK_SIZE / 2 + i * CHUNK_SIZE / 2 + k);
            }
        }
        Scramble(numRows, unscrambleArray, hmmArray);
        for (int j = 0; j < numRows; j += 2) {
            for (int k = 0; k < CHUNK_SIZE; k++) {
                if ((j / 2) % 2 == 0) {
                    *(hexArray + j * numChunks * CHUNK_SIZE + (i * CHUNK_SIZE) + k) = *(unscrambleArray + k * numRows / 2 + j / 2);
                    *(hexArray + (j + 1) * numChunks * CHUNK_SIZE + (i * CHUNK_SIZE) + k) = *(unscrambleArray + k * numRows / 2 + j / 2 + 1);
                }
                else {
                    *(hexArray + j * numChunks * CHUNK_SIZE + (i * CHUNK_SIZE) + k) = *(unscrambleArray + (k + 32) * numRows / 2 + j / 2 - 1);
                    *(hexArray + (j + 1) * numChunks * CHUNK_SIZE + (i * CHUNK_SIZE) + k) = *(unscrambleArray + (k + 32) * numRows / 2 + j / 2);
                }
            }
        }
    }
}

void writeImgFile(int numRows, int numChunks, unsigned char* bmpHeader, std::string filename, unsigned char palette[256][3],
    unsigned char* finalArray) {
    std::ofstream imgConverted;
    std::string outFilePath = filename;
    imgConverted.open(outFilePath, std::ios::binary);
    imgConverted.write((char*)bmpHeader, 0x36);
    for (int i = 0; i < numRows * 2; i++) {
        for (int j = 0; j < numChunks * CHUNK_SIZE / 2; j++) {
            imgConverted.write((char*)palette[*(finalArray + i * numChunks * CHUNK_SIZE / 2 + j)], 3);
        }
    }

    imgConverted.close();
}

void writeImgRaw(int width, int height, unsigned char* bmpHeader, std::string filename, unsigned char palette[256][3],
    unsigned char* finalArray) {
    std::ofstream imgConverted;
    std::string outFilePath = filename;
    imgConverted.open(outFilePath, std::ios::binary);
    imgConverted.write((char*)bmpHeader, 0x36);
    for (int i = 0; i < height; i++) {
        for (int j = 0; j < width; j++) {
            imgConverted.write((char*)palette[*(finalArray + i * width + j)], 3);
        }
    }

    imgConverted.close();
}

void writeRaw(int numRows, int numChunks, char* bmpHeader, std::string filename, unsigned char palette[256][3],
    unsigned char* hexArray) {
    std::ofstream imgConverted;
    std::string outFilePath = filename;
    imgConverted.open(outFilePath, std::ios::binary);
    imgConverted.write(bmpHeader, 0x36);
    char paletteReversed[256][3];
    for (int i = 0; i < 256; i++) {
        paletteReversed[i][0] = palette[i][2];
        paletteReversed[i][1] = palette[i][1];
        paletteReversed[i][2] = palette[i][0];
    }
    for (int i = 0; i < numRows; i++) {
        for (int j = 0; j < numChunks * CHUNK_SIZE; j++) {
            imgConverted.write((char*)paletteReversed[*(hexArray + i * numChunks * CHUNK_SIZE + j)], 3);
        }
    }
    char b[1] = { -1 };
    for (int c = 0; c < 196608; c++) {
        imgConverted.write(b, 1);
    }
    imgConverted.close();
}

void writeGameFile24Bit(int numRows, int numChunks, std::string fileName, unsigned char palette[256][3], unsigned char* imgArray, int offset) {
    std::fstream imgConverted;
    unsigned char paddingByte[1] = { 0x7F };
    imgConverted.open(fileName, std::ios::binary | std::ios::out | std::ios::in);
    imgConverted.seekp(offset);
    imgConverted.write((const char*)imgArray, numRows * numChunks * CHUNK_SIZE);
    unsigned char temp[3];
    for (int i = 8; i < 247; i++) {  //change back to 247
        if (i % 16 == 0) {
            i += 24;
        }
        temp[0] = palette[i][0];
        temp[1] = palette[i][1];
        temp[2] = palette[i][2];
        palette[i][0] = palette[i + 8][0];
        palette[i][1] = palette[i + 8][1];
        palette[i][2] = palette[i + 8][2];
        palette[i + 8][0] = temp[0];
        palette[i + 8][1] = temp[1];
        palette[i + 8][2] = temp[2];
    }
    for (int i = 0; i < 256; i++) {
        imgConverted.write((const char*)palette[i], 3);
        imgConverted.write((const char*)paddingByte, 1);
    }
    imgConverted.close();
}

void writeGameFile16Bit(int numRows, int numChunks, std::string fileName, unsigned char palette[256][3],
    unsigned char* imgArray, int offset) {
    std::fstream imgConverted;
    unsigned char paletteRGB[256][2];
    imgConverted.open(fileName, std::ios::binary | std::ios::out | std::ios::in);
    imgConverted.seekp(offset);
    imgConverted.write((const char*)imgArray, numRows * numChunks * CHUNK_SIZE);
    unsigned char temp[3];
    for (int i = 8; i < 247; i++) {  //change back to 247
        if (i % 16 == 0) {
            i += 24;
        }
        temp[0] = palette[i][0];
        temp[1] = palette[i][1];
        temp[2] = palette[i][2];
        palette[i][0] = palette[i + 8][0];
        palette[i][1] = palette[i + 8][1];
        palette[i][2] = palette[i + 8][2];
        palette[i + 8][0] = temp[0];
        palette[i + 8][1] = temp[1];
        palette[i + 8][2] = temp[2];
    }
    for (int i = 0; i < 256; i++) {
        paletteRGB[i][1] = (((palette[i][2] >> 3) << 2) | (palette[i][1] >> 6)) | 0x80;
        paletteRGB[i][0] = ((palette[i][1] >> 3) << 5) | (palette[i][0] >> 3);
    }
    for (int i = 0; i < 256; i++) {
        imgConverted.write((const char*)paletteRGB[i], 2);
    }
    imgConverted.close();
}

void writePalette(char* bmpHeader, int fileNum, unsigned char palette[256][3]) {
    std::ofstream paletteWriter;
    std::string outFilePath = R"(C:\Users\35mil\Pictures\NiGHTS\)" + IntToString3Width(fileNum) + "palette.bmp";
    paletteWriter.open(outFilePath, std::ios::binary);
    paletteWriter.write(bmpHeader, 0x36);
    for (int i = 0; i < 256; i++) {
        paletteWriter.write((char*)palette[i], 3);
    }
    paletteWriter.close();
}


void FindOffset(std::string filename, std::vector<ImgSpec>& headerInfo) {
    std::ifstream reader;
    reader.open(filename, std::ios::binary);
    char line[16];
    char discard[96];
    bool found;
    int counter = 0;
    while (!reader.eof()) {
        found = true;
        reader.read(line, 16);
        for (char i = 0; i < 16; i++) {
            if (i == 0) {
                if ((unsigned char)line[i] != 0x06) {
                    found = false;
                    break;
                }
            }
            else if (i == 3) {
                if ((unsigned char)line[i] != 0x10) {
                    found = false;
                    break;
                }
            }
            else {
                if ((unsigned char)line[i] != 0x00) {
                    found = false;
                    break;
                }
            }
        }
        if (found) {
            break;
        }
        counter += 16;
    }

    ImgSpec currInfo;
    reader.read(discard, 16);
    counter += 16;

    bool endReached = false;
    bool doubleSize = false;
    while (!endReached) {
        reader.read(line, 16);
        if (line[7] == 0x00) {
            doubleSize = true;
        }
        else {
            doubleSize = false;
        }
        reader.read(discard, 16);
        reader.read(line, 16);
        if (doubleSize) {
            currInfo.width = *((int*)&line[0]) * 2;
            currInfo.height = *((int*)(&line[4])) * 2;
        }
        else {
            currInfo.width = *((int*)&line[0]);
            currInfo.height = *((int*)(&line[4]));
        }
        reader.read(discard, 80);
        reader.read(line, 16);
        if (line[7] == 0) {
            currInfo.smallColors = false;
        }
        else {
            currInfo.smallColors = true;
        }

        reader.read(discard, 64);
        reader.read(line, 16);

        if (line[0] != 0x02) {
            currInfo.useNibbles = false;
        }
        else {
            currInfo.useNibbles = true;
            if (doubleSize) {
                currInfo.height *= 2;
            }
        }

        reader.read(discard, 16);
        reader.read(line, 16);

        counter += 256;
        headerInfo.push_back(currInfo);
        if (*((int*)&line) == 0xFFFFFFFF) {
            endReached = true;
        }

    }

    int initOffset = (256 - counter % 256) + counter;
    for (int i = 0; i < headerInfo.size(); i++) {
        headerInfo.at(i).offset = initOffset;
        if (headerInfo.at(i).smallColors) {
            if (headerInfo.at(i).useNibbles) {
                initOffset += headerInfo.at(i).width * headerInfo.at(i).height / 2 + 32;
            }
            else {
                initOffset += headerInfo.at(i).width * headerInfo.at(i).height + 256 * 2;
            }
        }
        else {
            initOffset += headerInfo.at(i).width * headerInfo.at(i).height + 256 * 4;
        }
    }

}

int SetDirectories(std::string& homeDir, std::string& gameDir) {
    std::cout << "Enter a home directory (starts with C:\\): ";
    getline(std::cin, homeDir);
    std::ofstream homeWrite;
    homeWrite.open("homeDir.txt");
    
    while (!IsPathExist(homeDir)) {
        std::cout << "Error: The directory does not exist. Enter another, or q to quit: ";
        getline(std::cin, homeDir);
        if (homeDir == "q") {
            std::cout << "Quitting" << std::endl;
            homeWrite.close();
            return QUIT;
        }
        else {
            homeWrite.open("homeDir.txt");
            homeWrite << homeDir;
        }
    }
    homeWrite << homeDir;
    homeWrite.close();

    std::cout << "Enter a game directory (starts with C:\\): ";
    getline(std::cin, gameDir);
    std::ofstream gameWrite;
    gameWrite.open("gameDir.txt");
   
    while (!IsPathExist(gameDir)) {
        std::cout << "Error: The directory does not exist. Enter another, or q to quit: ";
        getline(std::cin, gameDir);
        if (homeDir == "q") {
            std::cout << "Quitting" << std::endl;
            gameWrite.close();
            return QUIT;
        }
        else {
            gameWrite.open("gameDir.txt");
        }
    }

    gameWrite << gameDir;
    gameWrite.close();

    if (homeDir.back() != '\\') {
        homeDir.push_back('\\');
    }
    if (gameDir.back() != '\\') {
        gameDir.push_back('\\');
    }
    if (IsPathExist(gameDir)) {
        std::ifstream testIStream;
        testIStream.open(gameDir + "DATNIGHTS.BIN", std::ios::binary);
        if (!testIStream.is_open()) {
            std::cout << "Warning: The game directory may be incorrect (could not locate DATNIGHTS.BIN). You may change the game directory at any time by entering \"c\" at the menu." << std::endl;
        }
        testIStream.close();
    }
    
    return 0;
}


int InitializeDirectories(std::string& homeDir, std::string& gameDir) {
    std::ifstream inStream;
    std::ofstream outStream;
    inStream.open("homeDir.txt");
    getline(inStream, homeDir);
    inStream.close();
    if (homeDir.empty()) {
        std::cout << "Error: The home directory has not been set. Set one now? (y/n): ";
        getline(std::cin, homeDir);
        if (homeDir == "y") {
            std::cout << "Enter a home directory (starts with C:\\): ";
            getline(std::cin, homeDir);
        }
        else {
            std::cout << "Quitting" << std::endl;
            return QUIT;
        }
    }
    while (!IsPathExist(homeDir)) {
        std::cout << "Error: The directory does not exist. Enter another, or q to quit: ";
        getline(std::cin, homeDir);
        if (homeDir == "q") {
            std::cout << "Quitting" << std::endl;
            return QUIT;
        }
    }
    outStream.open("homeDir.txt");
    outStream << homeDir;
    outStream.close();

    inStream.open("gameDir.txt");
    getline(inStream, gameDir);
    inStream.close();
    if (gameDir.empty()) {
        std::cout << "Error: The game directory has not been set. Use default? (y/n): ";
        getline(std::cin, gameDir);
        if (gameDir == "y") {
            std::cout << "Attempting to set game directory to\n" << R"(C:\Program Files (x86)\Steam\steamapps\common\NiGHTS Into Dreams\afs\)" << std::endl;
            outStream.open("gameDir.txt");
            outStream << R"(C:\Program Files (x86)\Steam\steamapps\common\NiGHTS Into Dreams\afs\)";
            gameDir = R"(C:\Program Files (x86)\Steam\steamapps\common\NiGHTS Into Dreams\afs\)";
        }
        else {
            std::cout << "Set custom game directory? (y/n): ";
            getline(std::cin, gameDir);
            if (gameDir == "y") {
                std::cout << "Enter a game directory (starts with C:\\): ";
                getline(std::cin, gameDir);
            }
            else {
                std::cout << "Quitting" << std::endl;
                outStream.close();
                return QUIT;
            }
        }
    }
    if (homeDir.back() != '\\') {
        homeDir.push_back('\\');
    }
    if (gameDir.back() != '\\') {
        gameDir.push_back('\\');
    }
    while (!IsPathExist(gameDir)) {
        std::cout << "Error: The directory does not exist. Enter another, or q to quit: ";
        getline(std::cin, gameDir);
        if (gameDir == "q") {
            std::cout << "Quitting" << std::endl;
            outStream.close();
            return QUIT;
        }
        else {
        }
    }
    outStream << gameDir;
    outStream.close();
    if (IsPathExist(gameDir)) {
        std::ifstream testIStream;
        testIStream.open(gameDir + "DATNIGHTS.BIN", std::ios::binary);
        if (!testIStream.is_open()) {
            std::cout << "Warning: The game directory may be incorrect (could not locate DATNIGHTS.BIN). You may change the game directory at any time by entering \"c\" at the menu." << std::endl;
        }
        testIStream.close();
    }
    
    
    return 0;
}

int InputTexture(std::string& inFilePath, std::string& outFilePath, std::string& inFileName, std::vector<ImgSpec>& headerInfo) {
    std::string tempString;
    int index = -1;
    std::cout << "Enter an image index to write: ";
    getline(std::cin, tempString);
    std::fstream testStream;
    std::stringstream tempStream;
    tempStream << tempString;
    tempStream >> index;
    while (index < 0 || index >= headerInfo.size() || headerInfo.at(index).useNibbles) {
        if (index < 0 || index >= headerInfo.size()) {
            std::cout << "That index is out of range. Enter another image index or q to quit: ";
        }
        else if (headerInfo.at(index).useNibbles) {
            std::cout << "Writing 4-bit images is currently unsupported. Enter another image index or q to quit: ";
        }
        getline(std::cin, tempString);
        if (tempString == "q") {
            std::cout << "Quitting" << std::endl;
            return QUIT;
        }
        std::stringstream tempStream;
        tempStream << tempString;
        tempStream >> index;
    }


    int height = headerInfo.at(index).height;
    int width = headerInfo.at(index).width;
    int numRows = height / 2;
    int numChunks = width / 16;
    unsigned char paletteRGB[256][3];
    unsigned char* unscrambleArray = (unsigned char*)malloc(CHUNK_SIZE * numRows);
    unsigned char* hmmArray = (unsigned char*)malloc(CHUNK_SIZE * numRows);
    char* hexArray = (char*)malloc(numRows * numChunks * CHUNK_SIZE);
    unsigned char* finalArray = (unsigned char*)malloc(numRows * numChunks * CHUNK_SIZE);
    int imgSize = 0;

    std::cout << "Enter an image filename in the current directory \n(" << outFilePath << ") (case-sensitive): ";
    do {
        
        std::getline(std::cin, tempString);
        testStream.open(outFilePath + tempString, std::ios::in);
        while (!testStream.is_open()) {
            std::cout << "The file could not be opened. Enter another filename, or q to quit: ";
            std::getline(std::cin, tempString);
            if (tempString == "q") {
                std::cout << "Quitting" << std::endl;
                testStream.close();
                return QUIT;
            }
            else {
                testStream.open(outFilePath + tempString, std::ios::out);
            }
        }
        imgSize = GetBMPSize(outFilePath + tempString);
        if (imgSize != height * width) {
            testStream.close();
            std::cout << "Error: file size is not equal. Enter another filename or q to quit: ";
            getline(std::cin, tempString);

            if (tempString == "q") {
                std::cout << "Quitting" << std::endl;
                testStream.close();
                return QUIT;
            }
        }

    } while (imgSize != height * width);

    loadBMPFile(numRows, numChunks, outFilePath + tempString, paletteRGB, finalArray);

    revertArray(numRows, numChunks, unscrambleArray, hexArray, hmmArray, finalArray);
    if (headerInfo.at(index).smallColors) {
        writeGameFile16Bit(numRows, numChunks, inFilePath + inFileName, paletteRGB, (unsigned char*)hexArray, headerInfo.at(index).offset);
    }
    else {
        writeGameFile24Bit(numRows, numChunks, inFilePath + inFileName, paletteRGB, (unsigned char*)hexArray, headerInfo.at(index).offset);
    }
    testStream.close();
    std::cout << inFileName << " successfully written." << std::endl;

    free(hexArray);
    free(unscrambleArray);
    free(hmmArray);
    free(finalArray);

    return 0;
}

void OutputImages(std::string& inFilePath, std::string& outFilePath, std::string& inFileName, std::vector<ImgSpec>& headerInfo) {
    for (unsigned int i = 0; i < headerInfo.size(); i++) {
        std::fstream testStream;
        int offset = headerInfo.at(i).offset;
        int height = headerInfo.at(i).height;
        int width = headerInfo.at(i).width;
        int numRows = height / 2;
        int numChunks = width / 16;
        int paletteOffset = width * height;
        if (headerInfo.at(i).useNibbles) {
            paletteOffset /= 2;
        }

        int bmpNumBytes = width * height + 0x36;
        unsigned char* bmpNumBytesP = (unsigned char*)&bmpNumBytes;

        unsigned char* heightP = (unsigned char*)&height;
        unsigned char* widthP = (unsigned char*)&width;


        unsigned char bmpHeader[0x36] = { 0x42, 0x4D, *bmpNumBytesP, *(bmpNumBytesP + 1), *(bmpNumBytesP + 2),
                                         *(bmpNumBytesP + 3),
                                         0x00, 0x00, 0x00, 0x00, 0x36, 0x00, 0x00, 0x00, 0x28, 0x00, 0x00, 0x00,
                                         *widthP, *(widthP + 1), *(widthP + 2), *(widthP + 3),
                                         *heightP, *(heightP + 1), *(heightP + 2), *(heightP + 3),
                                         0x01, 0x00, 0x18, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x0C, 0x00, 0x00,
                                         0x00, 0x00,
                                         0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00 };

        std::string tempString;
        for (unsigned int i = 0; i < inFileName.size() - 4; i++) {
            tempString.push_back(inFileName.at(i));
        }
        std::string outFileName;

        if (headerInfo.size() == 1) {
            outFileName = tempString + ".bmp";
        }
        else {
            outFileName = tempString + "-" + IntToString3Width(i) + ".bmp";
        }
        
        testStream.open(outFilePath + outFileName, std::ios::out);

        unsigned char paletteRGB[256][3];
        unsigned char* unscrambleArray = (unsigned char*)malloc(CHUNK_SIZE * height);//numRows);
        unsigned char* hmmArray = (unsigned char*)malloc(CHUNK_SIZE * height);//numRows);
        char* hexArray = nullptr;
        unsigned char* finalArray = nullptr;
        unsigned char* splitFinalArray = (unsigned char*)malloc(width * height);
        if (headerInfo.at(i).useNibbles) {
            hexArray = (char*)malloc(width * height / 2);
            finalArray = (unsigned char*)malloc(width * height / 2);
        }
        else {
            hexArray = (char*)malloc(numRows * numChunks * CHUNK_SIZE);
            finalArray = (unsigned char*)malloc(numRows * numChunks * CHUNK_SIZE);
        }


        if (headerInfo.at(i).smallColors) {
            Palette16Bit(paletteRGB, inFilePath + inFileName, offset, paletteOffset);
        }
        else {
            Palette24Bit(paletteRGB, inFilePath + inFileName, offset, paletteOffset);
        }

        if (headerInfo.at(i).useNibbles) {
            loadGameFile8Bit(height, width / 2, inFilePath + inFileName, hexArray, paletteRGB, offset);
        }

        else {
            loadGameFile8Bit(height, width, inFilePath + inFileName, hexArray, paletteRGB, offset);
        }






        if (headerInfo.at(i).useNibbles) {
            //convertArray(numRows, numChunks/2, unscrambleArray, hexArray, hmmArray, finalArray);
            WidenArray((unsigned char*)hexArray, splitFinalArray, width * height);
            writeImgRaw(width, height, bmpHeader, outFilePath + outFileName, paletteRGB, splitFinalArray);
        }
        else {
            convertArray(numRows, numChunks, unscrambleArray, hexArray, hmmArray, finalArray);
            writeImgFile(numRows, numChunks, bmpHeader, outFilePath + outFileName, paletteRGB, finalArray);
        }

        free(splitFinalArray);
        free(hexArray);
        free(unscrambleArray);
        free(hmmArray);
        free(finalArray);


    }
    std::cout << inFileName << " successfully extracted." << std::endl;
}

void GenHeaderInfo(std::string inFilePath, std::string inFileName, std::vector<ImgSpec>& headerInfo) {
    headerInfo.clear();
    FindOffset(inFilePath + inFileName, headerInfo);
    std::cout << headerInfo.size() << " images detected." << std::endl;
    for (int i = 0; i < headerInfo.size(); i++) {
        std::cout << "Image " << i << ":" << std::endl;
        std::cout << "    Offset (bytes) in " << inFileName << ": " << headerInfo.at(i).offset << std::endl;
        std::cout << "    Size: " << headerInfo.at(i).width << "x" << headerInfo.at(i).height << std::endl;
        if (headerInfo.at(i).smallColors) {
            std::cout << "    Color format: 16-bit" << std::endl;
        }
        else {
            std::cout << "    Color format: 24-bit" << std::endl;
        }

        if (headerInfo.at(i).useNibbles) {
            std::cout << "    Pixel encoding: 4-bit (Note: 4-bit image conversion not yet implemented)" << std::endl;
        }
        else {
            std::cout << "    Pixel encoding: 8-bit" << std::endl;
        }
    }
}