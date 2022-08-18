#include <iostream>
#include <fstream>
#include <sstream>
#include <vector>
#include <iomanip>
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
void loadGameFile8Bit(int numRows, int width, std::string fileName, char* array, unsigned char palette[256][3], int offset, bool switchColorBlocks);
void loadGameFile16Bit(int numRows, int width, std::string fileName, unsigned short* array, int offset);
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
void OutputModels(std::string& inFilePath, std::string& outFilePath, std::string& inFileName, int numMats);
void GenHeaderInfo(std::string inFilePath, std::string inFileName, std::vector<ImgSpec>& headerInfo);
void convert4Bit(unsigned char* array);
void convert32x32(unsigned char* array1);




int main() {
    std::string modestr;   //user input
    char mode = '\0';
    std::vector<ImgSpec> headerInfo;  //header data from file
    std::string fileID;    //game file name
    std::string homeDir;   //output directory
    std::string gameDir;   //input/game directory

    while (mode != 'q') {
        std::cout << "Enter a mode:" << std::endl;
        std::cout << "    i: input an image to a game file" << std::endl;
        std::cout << "    o: output a game file's models and textures to the home directory" << std::endl;      //Print options menu
        std::cout << "    g: output all gallery images at once" << std::endl;
        std::cout << "    c: configure directories" << std::endl;
        std::cout << "    q: quit" << std::endl;
        std::cout << ":";
        std::getline(std::cin, modestr);   //getline to be consistent and avoid messing with cin ignore, since later inputs need getline
        mode = modestr.at(0);              //Trying to avoid string compare issues by using the first char

        if (mode == 'c') {     //configure
            SetDirectories(homeDir, gameDir);
        }

        else if (mode == 'i' || mode == 'o' || mode == 'g') {    //if not configuring, load directory information
            InitializeDirectories(homeDir, gameDir);
        }

        else {    //reprint menu if user input is invalid
            continue;
        }


        if (mode == 'i') {    //input mode

            std::cout << "WARNING: THIS TOOL MAY PERMANENTLY ALTER AND POTENTIALLY CORRUPT GAME FILES. It is highly recommended to create a backup of \n";
            std::cout << gameDir << " \nbefore continuing. Continue? (y/n): ";    //warning
            std::string tempString;
            std::getline(std::cin, tempString);   //user input y or n, erring on the side of caution
            if (tempString != "y") {
                std::cout << "Quitting" << std::endl;
                return 0;
            }
            std::cout << "Enter a game filename (case-sensitive): ";
            std::getline(std::cin, fileID);    //get game filename
            std::fstream testStream;
            testStream.open(gameDir + fileID);    //test if file can be opened
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
            //testStream.close();
            GenHeaderInfo(gameDir, fileID, headerInfo);   //once file can be opened, generate header info
            InputTexture(gameDir, homeDir, fileID, headerInfo);   //input texture to file
            std::cout << std::endl;
        }


        if (mode == 'o') {
            std::cout << "Enter a game filename (case-sensitive): ";
            std::getline(std::cin, fileID);
            std::fstream testStream;
            testStream.open(gameDir + fileID);    //test if file can be opened
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
            GenHeaderInfo(gameDir, fileID, headerInfo);   //once file can be opened, generate header info
            OutputModels(gameDir, homeDir, fileID, headerInfo.size()); //output all detected models to homeDir            
            OutputImages(gameDir, homeDir, fileID, headerInfo);   //output all detected images to homeDir
            std::cout << std::endl;
        }

        if (mode == 'g') {   //output all gallery images in sequence
            std::ifstream testIStream;
            testIStream.open(gameDir + "GAMEUI_GALLERY_001.BIN");  //since file names are predetermined, check if one exists in the gameDir
            if (!testIStream.is_open()) {
                std::cout << "Error: the gallery files could not be opened. Try checking your game directory." << std::endl;
                testIStream.close();
                continue;
            }
            testIStream.close();
            std::string fileNameList[8] = { "A01", "A02", "A03", "A04", "B01", "B02", "B03", "B04" };  //for non-numeric gallery file names
            for (int i = 1; i < 146; i++) {       //iterate through all numeric gallery files
                fileID = "GAMEUI_GALLERY_" + IntToString3Width(i) + ".BIN";
                GenHeaderInfo(gameDir, fileID, headerInfo);
                OutputImages(gameDir, homeDir, fileID, headerInfo);
                std::cout << std::endl;
            }
            for (int i = 0; i < 8; i++) {       //iterate through all non-numeric gallery files
                fileID = "GAMEUI_GALLERY_" + fileNameList[i] + ".BIN";
                GenHeaderInfo(gameDir, fileID, headerInfo);
                OutputImages(gameDir, homeDir, fileID, headerInfo);
                std::cout << std::endl;
            }
        }
    }

    return 0;
}


void Unscramble(int numRows, unsigned char* array, unsigned char* resolveArray) {   //unscramble 32-column chunk of 8-bit game file
    int order[16] = { 0,4,8,12,1,5,9,13,2,6,10,14,3,7,11,15 };   //order that lines are stored in the chunk
    int indices[4] = { 0,0,0,0 };
    int indices2[4] = { 0,0,0,0 };
    for (int i = 0; i < (CHUNK_SIZE / 2); i++) {  //i = column number
        if (order[i] % 2 == 0) {    //save indices of matching quarter lines that make up each line index in order[]
            indices[1] = order[i];
            indices[0] = order[i] + 17;
            indices[2] = order[i] + 33;
            indices[3] = order[i] + 48;
            indices2[0] = order[i];    //need two indices arrays because reconstruction order depends on the column
            indices2[1] = order[i] + 17;
            indices2[3] = order[i] + 33;
            indices2[2] = order[i] + 48;
        }
        else {     //numbers are different depending on parity of index in order[]
            indices[1] = order[i];
            indices[0] = order[i] + 15;
            indices[2] = order[i] + 31;
            indices[3] = order[i] + 48;
            indices2[0] = order[i];
            indices2[1] = order[i] + 15;
            indices2[3] = order[i] + 31;
            indices2[2] = order[i] + 48;
        }
        for (int j = 0; j < numRows / 2; j += 2) {  //j = row number
            for (int k = 0; k < 4; k++) { //k = which quarter line
                if ((i / 4) % 2 == 0) {    //if i is 0-3 or 8-11, use indices2, else use indices
                    *(resolveArray + i * numRows * 2 + j * 4 + k * 2) = *(array + indices2[k] * numRows / 2 + j);
                    *(resolveArray + i * numRows * 2 + j * 4 + k * 2 + 1) = *(array + indices2[k] * numRows / 2 + j + 1);   //weave together the quarter lines
                }
                else {
                    *(resolveArray + i * numRows * 2 + j * 4 + k * 2) = *(array + indices[k] * numRows / 2 + j);
                    *(resolveArray + i * numRows * 2 + j * 4 + k * 2 + 1) = *(array + indices[k] * numRows / 2 + j + 1);
                }
            }
        }
    }
}

void Scramble(int numRows, unsigned char* toArray, unsigned char* fromArray) {    //exact reverse of the above function, to convert bmp image back to game file format
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
    for (int i = 0; i < arraySize / 2; i++) {
        *(hexArray + i * 2) = (*(imgArray + i) >> 4) & 0x0F;    //separate the nibbles in 4-bit images into separate bytes to handle them more easily
        *(hexArray + i * 2 + 1) = *(imgArray + i) & 0x0F;
    }
}

void Palette16Bit(unsigned char paletteRGB[256][3], std::string filename, int offset, int paletteOffset) {
    std::ifstream bits;
    bits.open(filename, std::ios::binary);   //treating data as binary to ignore bytes that correspond to newlines
    bits.seekg(offset + paletteOffset);    //go to the palette's location in data
    char bitColor[2] = { '\0','\0' };
    unsigned short bitVal = 0;
    int counter = 0;
    while (counter < 256) {
        bits.read(bitColor, 2);    //read two bytes as chars
        bitVal = *((unsigned short*)bitColor);   //casting to 2-byte value
        if ((bitVal & 0x8000) >> 15) {        //if color not transparent
            paletteRGB[counter][0] = (bitVal & 0x7c00) >> 7;    //separate the 5-bit colors into separate bytes
            paletteRGB[counter][1] = (bitVal & 0x03e0) >> 2;
            paletteRGB[counter][2] = (bitVal & 0x001f) << 3;
            counter++;
        }
        else {      //if transparent
            paletteRGB[counter][0] = 0x00;    //set to pure green for testing purposes
            paletteRGB[counter][1] = 0xFF;
            paletteRGB[counter][2] = 0x00;
            counter++;
        }
    }
}

void Palette24Bit(unsigned char paletteRGB[256][3], std::string filename, int offset, int paletteOffset) {
    std::ifstream bits;
    bits.open(filename, std::ios::binary);   //similar to the above
    bits.seekg(offset + paletteOffset);
    char bitColor[3] = { '\0','\0','\0' };
    int counter = 0;
    while (counter < 256) {
        bits.read(bitColor, 3);    //but no need for bit masking
        paletteRGB[counter][0] = bitColor[2];
        paletteRGB[counter][1] = bitColor[1];   //color order needs to be flipped because original order is BGR
        paletteRGB[counter][2] = bitColor[0];
        counter++;     
        bits.read(bitColor, 1);
    }
}

std::string IntToString3Width(int value) {   //to easily get 3-digit numbers as strings
    std::stringstream stream;
    stream << value;
    std::string numStr;
    stream >> numStr;
    while (numStr.size() < 3) {
        numStr = "0" + numStr;
    }
    if (value == 1000) {   //for testing purposes, since 1000 is already out of range for 3 digit integers
        return "nightTest";
    }
    return numStr;
}

int GetBMPSize(std::string fileName) {  //the first lines of the following function, to quickly find and return height*width from BMP header
    char discard[18];
    char widthData[4];
    char heightData[4];
    std::ifstream imgFile;
    imgFile.open(fileName, std::ios::binary);
    imgFile.read(discard, 18);   //skip irrelevant part of header
    imgFile.read(widthData, 4);
    imgFile.read(heightData, 4);
    return (*(int*)(&widthData) * *(int*)(&heightData));   //dereference height and width as ints and multiply
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
    int paddingSize = 4 - ((numChunks * CHUNK_SIZE / 2) * 3) % 4;  //number of padding bytes after each row of pixel data
    if (paddingSize == 4) {
        paddingSize = 0;
    }
    for (int i = 0; i < numRows * 2; i++) {
        for (int j = 0; j < numChunks * CHUNK_SIZE / 2; j++) {
            imgFile.read(pixel, 3);     //read in one pixel's data
            *(imgArray + i * numChunks * CHUNK_SIZE / 2 + j) = FindInPalette((unsigned char*)pixel, palette, paletteSize); //map this pixel to a color in the palette
        }
        imgFile.read(discard, paddingSize);  //skip any padding bytes
    }
    for (int i = paletteSize; i < 256; i++) {
        palette[i][0] = 0;
        palette[i][1] = 0;
        palette[i][2] = 0;  //fill empty spaces in the palette with #000000
    }

}

unsigned char FindInPalette(unsigned char pixel[3], unsigned char palette[256][3], short& paletteSize) {
    for (int i = 0; i < paletteSize; i++) {
        if (pixel[0] == palette[i][2] && pixel[1] == palette[i][1] && pixel[2] == palette[i][0]) {
            return i;     //if color already in palette, return index of color
        }
    }
    if (paletteSize < 256) {   //else, if there's space left in the palette, add the new color and return its index
        palette[paletteSize][2] = pixel[0];
        palette[paletteSize][1] = pixel[1];
        palette[paletteSize][0] = pixel[2];
        paletteSize++;
        return paletteSize - 1;
    }
    else {  //else if palette full, default to 0
        return 0;
    }
}

void loadGameFile8Bit(int numRows, int width, std::string fileName, char* array, unsigned char palette[256][3], int offset, bool switchColorBlocks) {
    std::ifstream imgFile;
    imgFile.open(fileName, std::ios::binary);
    char hex[1] = { 0 };
    unsigned char temp[3] = { 0,0,0 };
    int counter = 0;
    while (!imgFile.eof()) {
        imgFile.read(hex, 1);
        if (counter >= offset && counter < offset + (width * (numRows))) {
            *(array + counter - offset) = *hex;    //store bytes between the offset and end of the image data
        }
        else if (counter > offset + (width * (numRows))) {
            break;    //stop once this end is reached
        }
        counter++;
    }

    if (switchColorBlocks) {     //if necessary (8-bit images) unscramble the palette
        for (int i = 8; i < 247; i++) {
            if (i % 16 == 0) {
                i += 24;
            }
            temp[0] = palette[i][0];
            temp[1] = palette[i][1];
            temp[2] = palette[i][2];
            palette[i][0] = palette[i + 8][0];    //switches every other pair of 8-color chunks in the palette
            palette[i][1] = palette[i + 8][1];
            palette[i][2] = palette[i + 8][2];
            palette[i + 8][0] = temp[0];
            palette[i + 8][1] = temp[1];
            palette[i + 8][2] = temp[2];
        }
    }
    imgFile.close();
}

void loadGameFile16Bit(int numRows, int width, std::string fileName, unsigned short* array, int offset) {
    std::ifstream imgFile;
    imgFile.open(fileName, std::ios::binary);
    char hexes[2] = { 0, 0 };
    int counter = 0;
    imgFile.seekg(offset);
    while (!imgFile.eof()) {
        imgFile.read(hexes, 2);
        if (counter < (width * numRows)) {
            *(array + counter) = *(unsigned short *) hexes;    //store bytes between the offset and end of the image data
        }
        else {
            break;    //stop once this end is reached
        }
        counter += 1;
    }
    imgFile.close();
}

void convert32x32(unsigned char* array1) {
    unsigned char* tempArray = (unsigned char*)malloc(32 * 32);

    for (int i = 0; i < 16; i++) {     //rearrange 16 sets of 64 contiguous pixels, original order is  0 8 1 9 2 10 3 11 4 12 5 13 6 14 7 15
        for (int j = 0; j < 64; j++) {   // needs to be rearranged to 0 1 2 3 4 5 6 7 8 9 10 11 12 13 14 15
            if (i % 2 == 0) {
                *(tempArray + i / 2 * 64 + j) = *(array1 + i * 64 + j);
            }
            else {
                *(tempArray + i / 2 * 64 + j + 32 * 16) = *(array1 + i * 64 + j);
            }
        }
    }

    for (int i = 0; i < 32 * 32; i++) {  //load tempArray back into array
        *(array1 + i) = *(tempArray + i);
    }
    free(tempArray);

    for (int i = 0; i < 16; i++) {    //swap every other pair of pixels with the pair of pixels in the following row
        for (int j = 0; j < 32; j++) {
            if ((i / 2 % 2 == 0 && (j % 4 == 1 || j % 4 == 2)) || (i / 2 % 2 == 1 && (j % 4 == 0 || j % 4 == 3))) {
                unsigned char temp = *(array1 + (i * 2 + 1) * 32 + j);
                *(array1 + (i * 2 + 1) * 32 + j) = *(array1 + i * 2 * 32 + j);
                *(array1 + i * 2 * 32 + j) = temp;
            }
        }
    }
    tempArray = (unsigned char*)malloc(32 * 32);

    for (int i = 0; i < 32; i++) {    //put every 8th pixel in a 32-pixel row next to each other
        for (int j = 0; j < 32; j++) {
            *(tempArray + i * 32 + j) = *(array1 + i * 32 + (j % 4) * 8 + j / 4);
        }
    }
    for (int i = 0; i < 32 * 32; i++) {
        *(array1 + i) = *(tempArray + i);
    }

    for (int i = 0; i < 16; i++) {
        for (int j = 0; j < 4; j++) {
            for (int k = 0; k < 4; k++) {
                unsigned char temp = *(array1 + (i * 2 + 1) * 32 + j * 8 + 4 + k);  //"unweave" adjacent 4-pixel-wide columns 
                *(array1 + (i * 2 + 1) * 32 + j * 8 + 4 + k) = *(array1 + i * 2 * 32 + j * 8 + k);
                *(array1 + i * 2 * 32 + j * 8 + k) = temp;
            }
        }
    }

    for (int i = 0; i < 32; i++) {
        for (int j = 0; j < 4; j++) {
            for (int k = 0; k < 4; k++) {
                if (j % 2 == 1) {
                    unsigned char temp = *(array1 + i * 32 + j * 8 + 4 + k);  //switch odd pairs of columns
                    *(array1 + i * 32 + j * 8 + 4 + k) = *(array1 + i * 32 + j * 8 + k);
                    *(array1 + i * 32 + j * 8 + k) = temp;
                }
            }
        }
    }

    for (int i = 0; i < 8; i++) {
        for (int j = 0; j < 32; j++) {
            unsigned char temp = *(array1 + i * 128 + 32 + j);  //one more pass to swap every other pair of rows
            *(array1 + i * 128 + 32 + j) = *(array1 + i * 128 + 64 + j);
            *(array1 + i * 128 + 64 + j) = temp;
        }
    }

    free(tempArray);
}

void convert4Bit(unsigned char* array1) {   //unscramble 4-bit image data
    unsigned char* tempArray = (unsigned char*)malloc(32 * 32);

    /*for (int i = 0; i < 16; i++) {     //rearrange 16 sets of 64 contiguous pixels, original order is  0 8 1 9 2 10 3 11 4 12 5 13 6 14 7 15
        for (int j = 0; j < 64; j++) {   // needs to be rearranged to 0 1 2 3 4 5 6 7 8 9 10 11 12 13 14 15
            if (i % 2 == 0) {
                *(tempArray + i / 2 * 64 + j) = *(array1 + i * 64 + j);
            }
            else {
                *(tempArray + i / 2 * 64 + j + 32 * 16) = *(array1 + i * 64 + j);
            }
        }
    }
    for (int i = 0; i < 32 * 32; i++) {  //load tempArray back into array
        *(array1 + i) = *(tempArray + i);
    }
    free(tempArray);*/

    /*for (int i = 0; i < 16; i++) {    //swap every other pair of pixels with the pair of pixels in the following row
        for (int j = 0; j < 32; j++) {
            if ((i / 2 % 2 == 0 && (j % 4 == 1 || j % 4 == 2)) || (i / 2 % 2 == 1 && (j % 4 == 0 || j % 4 == 3))) {
                unsigned char temp = *(array1 + (i * 2 + 1) * 32 + j);
                *(array1 + (i * 2 + 1) * 32 + j) = *(array1 + i * 2 * 32 + j);
                *(array1 + i * 2 * 32 + j) = temp;
            }
        }
    }*/
    tempArray = (unsigned char*)malloc(32 * 32);

    /*for (int i = 0; i < 4; i++) {
        if (i % 2 == 1) {
            for (int j = 0; j < 16; j++) {
                for (int k = 0; k < 8; k++) {
                    unsigned char temp = *(array1 + (i * 32 * 8) + j * 2 + k * 32);
                    *(array1 + (i * 32 * 8) + j * 2 + k * 32) = *(array1 + (i * 32 * 8) + j * 2 + k * 32 + 1);
                    *(array1 + (i * 32 * 8) + j * 2 + k * 32 + 1) = temp;    //switch certain pixels
                }
            }
        }
    }*/

    for (int i = 0; i < 32; i++) {    //put every 8th pixel in a 32-pixel row next to each other
        for (int j = 0; j < 32; j++) {
            *(tempArray + i * 32 + j) = *(array1 + i * 32 + (j % 4) * 8 + j / 4);
        }
    }

    for (int i = 0; i < 32; i++) {    //interweave every other row
        for (int j = 0; j < 32; j++) {
            if (i % 2 == 0) {
                *(array1 + i * 32 + j) = *(tempArray + i / 8 * 8 * 32 + i % 8 / 2 * 32 + j);
            }
            else {
                *(array1 + i * 32 + j) = *(tempArray + i / 8 * 8 * 32 + i % 8 / 2 * 32 + 4 * 32 + j);
            }
        }
    }

    /*for (int i = 0; i < 16; i++) {
        for (int j = 0; j < 4; j++) {
            for (int k = 0; k < 4; k++) {
                unsigned char temp = *(array1 + (i * 2 + 1) * 32 + j * 8 + 4 + k);  //"unweave" adjacent 4-pixel-wide columns
                *(array1 + (i * 2 + 1) * 32 + j * 8 + 4 + k) = *(array1 + i * 2 * 32 + j * 8 + k);
                *(array1 + i * 2 * 32 + j * 8 + k) = temp;
            }
        }
    }*/

    /*for (int i = 0; i < 32; i++) {
        for (int j = 0; j < 4; j++) {
            for (int k = 0; k < 4; k++) {
                if (j % 2 == 1) {
                    unsigned char temp = *(array1 + i * 32 + j * 8 + 4 + k);  //switch odd pairs of columns
                    *(array1 + i * 32 + j * 8 + 4 + k) = *(array1 + i * 32 + j * 8 + k);
                    *(array1 + i * 32 + j * 8 + k) = temp;
                }
            }
        }
    }*/

    /*for (int i = 0; i < 8; i++) {
        for (int j = 0; j < 32; j++) {
            unsigned char temp = *(array1 + i * 128 + 32 + j);  //one more pass to swap every other pair of rows
            *(array1 + i * 128 + 32 + j) = *(array1 + i * 128 + 64 + j);
            *(array1 + i * 128 + 64 + j) = temp;
        }
    }*/

    free(tempArray);
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

void writeImgFile16Bit(int height, int width, unsigned char* bmpHeader, std::string filename, unsigned short* finalArray) {
    std::ofstream imgConverted;
    std::string outFilePath = filename;
    imgConverted.open(outFilePath, std::ios::binary);
    imgConverted.write((char*)bmpHeader, 0x36);
    unsigned char RGBpixel[3] = { 0, 0, 0 };
    unsigned short bitVal = 0;
    for (int i = 0; i < height*width; i++) {
        bitVal = *(finalArray + i);
        RGBpixel[0] = (bitVal & 0x7c00) >> 7;    //separate the 5-bit colors into separate bytes
        RGBpixel[1] = (bitVal & 0x03e0) >> 2;
        RGBpixel[2] = (bitVal & 0x001f) << 3;

        imgConverted.write((char*)RGBpixel, 3);
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

/*void writeRaw(int numRows, int numChunks, char* bmpHeader, std::string filename, unsigned char palette[256][3],
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
}*/

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
    while (!endReached && !reader.eof()) {
        reader.read(line, 16);
        /*if (line[6] > 4) {
            currInfo.pixelEncoding = SIXTEEN_BIT;
            currInfo.smallColors = false;
        }*/
        /*if (line[4] == 16 && line[5] == 2) {
            currInfo.pixelEncoding = SIXTEEN_BIT;
            currInfo.smallColors = false;
        }*/
        /*if (headerInfo.size() == 195 || headerInfo.size() == 197) {
            currInfo.pixelEncoding = SIXTEEN_BIT;
            currInfo.smallColors = false;
        }*/ //for DITEM_XMAS_PIA_PS2.BIN
        /*if (headerInfo.size() == 51 || headerInfo.size() == 52 || headerInfo.size() == 131 || headerInfo.size() == 132 || headerInfo.size() == 133 || headerInfo.size() == 212) {
            currInfo.pixelEncoding = SIXTEEN_BIT;
            currInfo.smallColors = false;
        }*/ //for DITEM_COM_PS2.BIN
        
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
        if (currInfo.pixelEncoding == SIXTEEN_BIT) {
            currInfo.height /= 2;
            reader.read(discard, 64);
            reader.read(line, 16);
            counter += 128;
            headerInfo.push_back(currInfo);
            currInfo.pixelEncoding = EIGHT_BIT;
            if (*((int*)&line) == 0xFFFFFFFF) {
                endReached = true;
            }
            continue;
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

        if (line[0] == 0x02)  {
            currInfo.pixelEncoding = FOUR_BIT;
            if (doubleSize) {
                currInfo.height *= 2;
            }
        }

        reader.read(discard, 16);
        reader.read(line, 16);

        counter += 256;
        headerInfo.push_back(currInfo);
        currInfo.pixelEncoding = EIGHT_BIT;
        if (*((int*)&line) == 0xFFFFFFFF) {
            endReached = true;
        }

    }

    int initOffset = (256 - counter % 256) + counter;
    for (int i = 0; i < headerInfo.size(); i++) {
        headerInfo.at(i).offset = initOffset;
        if (headerInfo.at(i).pixelEncoding == SIXTEEN_BIT) {
            initOffset += headerInfo.at(i).width * headerInfo.at(i).height * 2;
        }
        else if (headerInfo.at(i).smallColors) {                 //FIX THIS 
            if (headerInfo.at(i).pixelEncoding == FOUR_BIT) {
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
    while (index < 0 || index >= headerInfo.size() || headerInfo.at(index).pixelEncoding == FOUR_BIT) {           //Keep asking for input until the input is valid (not out of range and not 4-bit)
        if (index < 0 || index >= headerInfo.size()) {
            std::cout << "That index is out of range. Enter another image index or q to quit: ";
        }
        else if (headerInfo.at(index).pixelEncoding == FOUR_BIT) {
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
    std::fstream testStream;
    std::string fileName2 = inFileName;
    if (((char)fileName2.at(0) == 'p' || (char)fileName2.at(0) == 'P') && ((char)fileName2.at(1) == 'n' || (char)fileName2.at(1) == 'N')) {
        if ((char)fileName2.at(0) == 'p') {
            fileName2.at(0) = 't';
        }
        else {
            fileName2.at(0) = 'T';
        }
    }
    for (unsigned int i = 0; i < headerInfo.size(); i++) {
        int offset = headerInfo.at(i).offset;
        int height = headerInfo.at(i).height;
        int width = headerInfo.at(i).width;
        int numRows = height / 2;
        int numChunks = width / 16;
        int paletteOffset = width * height;
        if (headerInfo.at(i).pixelEncoding == FOUR_BIT) {
            paletteOffset /= 2;
        }

        int bmpNumBytes = width * height + 0x36;
        unsigned char* bmpNumBytesP = (unsigned char*)&bmpNumBytes;

        unsigned char* heightP = (unsigned char*)&height;
        unsigned char* widthP = (unsigned char*)&width;

        /*if (headerInfo.at(i).useNibbles) {
            heightP = (unsigned char*)&width;
            widthP = (unsigned char*)&height;
        }*/
        unsigned char bmpHeader[0x36] = { 0x42, 0x4D, *bmpNumBytesP, *(bmpNumBytesP + 1), *(bmpNumBytesP + 2),
                                         *(bmpNumBytesP + 3),
                                         0x00, 0x00, 0x00, 0x00, 0x36, 0x00, 0x00, 0x00, 0x28, 0x00, 0x00, 0x00,
                                         *widthP, *(widthP + 1), *(widthP + 2), *(widthP + 3),
                                         *heightP, *(heightP + 1), *(heightP + 2), *(heightP + 3),
                                         0x01, 0x00, 0x18, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x0C, 0x00, 0x00,
                                         0x00, 0x00,
                                         0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00 };

        std::string tempString;
        for (unsigned int i = 0; i < fileName2.size() - 4; i++) {
            tempString.push_back(fileName2.at(i));
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

        if (headerInfo.at(i).pixelEncoding == SIXTEEN_BIT) {
            unsigned short* shortArray = (unsigned short*)malloc(width * height * 2);
            unsigned short* shortArray2 = (unsigned short*)malloc(width * height * 2);
            loadGameFile16Bit(height, width, inFilePath + fileName2, shortArray, offset);
            for (int i = 0; i < height; i++) {
                for (int j = 0; j < width/16; j++) {
                    for (int k = 0; k < 16; k++) {
                        *(shortArray2 + i * width + j*16 + k % 2 * 8 + k / 2) = *(shortArray + i * width + j * 16 + k);
                    }
                }
            }
            for (int i = 0; i < height; i++) {
                for (int j = 0; j < width; j++) {
                    *(shortArray + i * width + j) = *(shortArray2 + i * width + j);
                }
            }
            writeImgFile16Bit(height, width, bmpHeader, outFilePath + outFileName, shortArray);
            free(shortArray);
            free(shortArray2);
        }

        else {
            if (headerInfo.at(i).smallColors) {
                Palette16Bit(paletteRGB, inFilePath + fileName2, offset, paletteOffset);
            }
            else {
                Palette24Bit(paletteRGB, inFilePath + fileName2, offset, paletteOffset);
            }
        }


        if (headerInfo.at(i).pixelEncoding == FOUR_BIT) {
            hexArray = (char*)malloc(width * height / 2);
            finalArray = (unsigned char*)malloc(width * height / 2);
            loadGameFile8Bit(height, width / 2, inFilePath + fileName2, hexArray, paletteRGB, offset, false);
            unsigned char* chunkArray = (unsigned char*)malloc(CHUNK_SIZE * CHUNK_SIZE);
            unsigned char* newArray = (unsigned char*)malloc(width * height);
            WidenArray((unsigned char*)hexArray, splitFinalArray, width * height);
            char temp;
            if (width == 64 && height == 64) {
                for (int i = 0; i < height; i++) {
                    if (i / 8 % 2 == 1) {
                        for (int j = 0; j < width / 4; j++) {
                            temp = *(splitFinalArray + i * width + j * 2 + 1);
                            *(splitFinalArray + i * width + j * 2 + 1) = *(splitFinalArray + i * width + width / 2 + j * 2);
                            *(splitFinalArray + i * width + width / 2 + j * 2) = *(splitFinalArray + i * width + width / 2 + j * 2 + 1);
                            *(splitFinalArray + i * width + width / 2 + j * 2 + 1) = temp;
                        }
                    }
                    else {
                        for (int j = 0; j < width / 4; j++) {
                            temp = *(splitFinalArray + i * width + width / 2 + j * 2);
                            *(splitFinalArray + i * width + width / 2 + j * 2) = *(splitFinalArray + i * width + j * 2 + 1);
                            *(splitFinalArray + i * width + j * 2 + 1) = *(splitFinalArray + i * width + j * 2);
                            *(splitFinalArray + i * width + j * 2) = temp;
                        }
                    }
                }
                for (int w = 0; w < width / CHUNK_SIZE; w++) {
                    for (int h = 0; h < height / CHUNK_SIZE; h++) {
                        for (int c = 0; c < CHUNK_SIZE; c++) {
                            for (int d = 0; d < CHUNK_SIZE; d++) {
                                *(chunkArray + c * CHUNK_SIZE + d) = *(splitFinalArray + h * width * CHUNK_SIZE + c * width + w * CHUNK_SIZE + d);
                            }
                        }
                        convert4Bit(chunkArray);
                        for (int c = 0; c < CHUNK_SIZE; c++) {
                            for (int d = 0; d < CHUNK_SIZE; d++) {
                                *(splitFinalArray + h * width * CHUNK_SIZE + c * width + w * CHUNK_SIZE + d) = *(chunkArray + c * CHUNK_SIZE + d);
                            }
                        }
                    }
                }
                for (int i = 0; i < 64; i++) {
                    for (int j = 0; j < 32; j++) {
                        if (i % 2 == 1) {
                            *(newArray + (i - 1) * 2 * 32 + j + 32) = *(splitFinalArray + i % 16 / 4 * 16 * 32 + i / 16 * 4 * 32 + (1 - i / 2 % 2) * 32 + 32 * 64 + j);
                            *(newArray + (i - 1) * 2 * 32 + j + 32 + 64) = *(splitFinalArray + i % 16 / 4 * 16 * 32 + i / 16 * 4 * 32 + (1 - i / 2 % 2) * 32 + 32 * 64 + j + 64);
                        }
                        else {
                            *(newArray + i * 2 * 32 + j) = *(splitFinalArray + i % 16 / 4 * 16 * 32 + i / 16 * 4 * 32 + (1 - i / 2 % 2) * 32 + j);
                            *(newArray + i * 2 * 32 + j + 64) = *(splitFinalArray + i % 16 / 4 * 16 * 32 + i / 16 * 4 * 32 + (1 - i / 2 % 2) * 32 + j + 64);
                        }
                    }
                }
                free(splitFinalArray);
                splitFinalArray = newArray;
            }


            else if (width == 32) {
                for (int h = 0; h < height / 32; h++) {
                    convert32x32(splitFinalArray + 32 * 32 * h);
                }
            }

            else if (width <= 16) {
                for (int i = 0; i < height; i++) {
                    for (int j = 0; j < width; j += 2) {
                        unsigned char temp = splitFinalArray[i * width + j];
                        splitFinalArray[i * width + j] = splitFinalArray[i * width + j + 1];
                        splitFinalArray[i * width + j + 1] = temp;
                    }
                }
            }

            writeImgRaw(width, height, bmpHeader, outFilePath + outFileName, paletteRGB, splitFinalArray);
            free(chunkArray);
            free(hexArray);
            free(finalArray);
        }
        
        else if (headerInfo.at(i).pixelEncoding == EIGHT_BIT) {
            hexArray = (char*)malloc(width*height);
            finalArray = (unsigned char*)malloc(width * height);
            loadGameFile8Bit(height, width, inFilePath + fileName2, hexArray, paletteRGB, offset, true);
                        
            if (width >= CHUNK_SIZE) {
                convertArray(numRows, numChunks, unscrambleArray, hexArray, hmmArray, finalArray);
                writeImgFile(numRows, numChunks, bmpHeader, outFilePath + outFileName, paletteRGB, finalArray);
            }
            else {
                writeImgRaw(width, height, bmpHeader, outFilePath + outFileName, paletteRGB, (unsigned char*)hexArray);
            }

            free(hexArray);
            free(finalArray);
        }

        free(splitFinalArray);
        free(unscrambleArray);
        free(hmmArray);
        

    }
    testStream.close();
    std::cout << headerInfo.size() << " images extracted from " << fileName2 << "." << std::endl;
    std::cout << std::endl;
}

void OutputModels(std::string& inFilePath, std::string& outFilePath, std::string& inFileName, int numMats) {
    enum print_mode { VERTEX, TEXTURE_MAP, NO_PRINT, VERTEX_NORMAL };
    print_mode shouldPrint = NO_PRINT;

    char discard[44];
    char dword[4];

    float floatVal = 0.0;
    float vertexXYZ[3];
    float vertexNorm[3];
    float textureMapXY[2];

    int expectedVertices = 0;
    int numVertices = 0;
    int numTotalVerts = 0;
    int totalVertGroups = 0;
    int numPolygons = 0;
    int currVertIndex = 0;
    int currTexMapIndex = 0;
    int objectNum = 0;
    int currVertGroup = 0;

    std::string fileName;
    std::string fileName2;
    for (unsigned int i = 0; i < inFileName.size() - 4; i++) {
        fileName.push_back(inFileName.at(i));
    }
    fileName2 = fileName;
    if (((char)fileName2.at(0) == 'p' || (char)fileName2.at(0) == 'P') && ((char)fileName2.at(1) == 'n' || (char)fileName2.at(1) == 'N')) {
        if ((char)fileName2.at(0) == 'p') {
            fileName2.at(0) = 't';
        }
        else {
            fileName2.at(0) = 'T';
        }
    }

    std::ifstream reader;
    std::ofstream writer;
    std::ofstream mtlWriter;

    reader.open(inFilePath + inFileName, std::ios::binary);
    writer.open(outFilePath + fileName + "-models.obj");
    mtlWriter.open(outFilePath + fileName + ".mtl");
    writer << "mtllib " << fileName << ".mtl" << std::endl;
    while (!reader.eof()) {
        reader.read(dword, 4);
        if (!((unsigned char)dword[0] == 0x00 && (unsigned char)dword[1] == 0x10 &&
            (unsigned char)dword[2] == 0x00 && (unsigned char)dword[3] == 0x00)) {
            break;
        }
        writer << "o " << fileName << "-" << IntToString3Width(objectNum) << std::endl;
        printf("Offset: %x\n", (int)reader.tellg());
        reader.read(discard, 4);
        reader.read(dword, 4);
        expectedVertices = *(unsigned int*)dword;
        std::stringstream vertices;
        std::stringstream textureVerts;
        std::stringstream faces;
        while (numVertices < expectedVertices) {
            reader.read(dword, 4);

            if ((unsigned char)dword[0] == 0x05 && (unsigned char)dword[1] == 0x80 &&
                (unsigned char)dword[3] == 0x7E) {
                
                shouldPrint = VERTEX_NORMAL;
                reader.read(dword, 4);
            }
            if ((unsigned char)dword[0] == 0x06 && (unsigned char)dword[1] == 0xC0 &&
                (unsigned char)dword[3] == 0x6E) {
                shouldPrint = NO_PRINT;
            }
            if ((unsigned char)dword[0] == 0x04 && (unsigned char)dword[1] == 0x04 &&
                (unsigned char)dword[2] == 0x00 && (unsigned char)dword[3] == 0x01) {
                shouldPrint = NO_PRINT;
                totalVertGroups++;
            }

            floatVal = *(float*)dword;
            vertexNorm[0] = (float)dword[0];
            vertexNorm[1] = (float)dword[1];
            vertexNorm[2] = (float)dword[2];
            vertexXYZ[currVertIndex % 3] = floatVal;
            currVertIndex++;
            textureMapXY[currTexMapIndex % 2] = floatVal;
            currTexMapIndex++;

            if (currVertIndex % 3 == 0 && shouldPrint == VERTEX) {
                vertices << std::setprecision(6) << std::fixed << "v " << vertexXYZ[0] << " " << vertexXYZ[1] << " " << vertexXYZ[2] << std::endl;
            }
            if (shouldPrint == VERTEX_NORMAL) {
                //vertices << std::setprecision(6) << std::fixed << "vn " << vertexNorm[0] << " " << vertexNorm[1] << " " << vertexNorm[2] << std::endl;
            }
            if (currTexMapIndex % 2 == 0 && shouldPrint == TEXTURE_MAP) {
                numVertices++;
                numTotalVerts++;
                textureVerts << std::setprecision(6) << std::fixed << "vt " << textureMapXY[0] << " " << textureMapXY[1] << std::endl;
            }

            if ((unsigned char)dword[0] == 0x07 && (unsigned char)dword[1] == 0x80 &&
                (unsigned char)dword[3] == 0x64) {
                shouldPrint = TEXTURE_MAP;
                currTexMapIndex = 0;
            }


            if ((unsigned char)dword[0] == 0xFE && (unsigned char)dword[1] == 0xFF &&
                (unsigned char)dword[2] == 0xFF && (unsigned char)dword[3] == 0xFF) {
                reader.read(dword, 4);
                faces << "usemtl m" << *(unsigned int *)dword / 2 << std::endl;
                mtlWriter << "newmtl m" << *(unsigned int*)dword / 2 << std::endl;
                if (numMats == 1) {
                    mtlWriter << "map_Kd " << outFilePath << fileName2 << ".bmp" << std::endl;
                }
                else {
                    mtlWriter << "map_Kd " << outFilePath << fileName2 << "-" << IntToString3Width(*(unsigned int*)dword / 2) << ".bmp" << std::endl;
                }
                currVertGroup++;
            }

            if ((unsigned char)dword[0] == 0x04 && (unsigned char)dword[1] == 0x80 &&
                (unsigned char)dword[3] == 0x78) {
                for (int i = 0; i < (unsigned char)dword[2] - 2; i++) {
                    if (i % 2) {
                        faces << "f " << numTotalVerts + 1 + i << "/" << numTotalVerts + 1 + i << "/" << numTotalVerts + 1 + i << " ";
                        faces << numTotalVerts + 2 + i << "/" << numTotalVerts + 2 + i << "/" << numTotalVerts + 2 + i << " ";
                        faces << numTotalVerts + 3 + i << "/" << numTotalVerts + 3 + i << "/" << numTotalVerts + 3 + i << std::endl;
                    }
                    else {
                        faces << "f " << numTotalVerts + 3 + i << "/" << numTotalVerts + 3 + i << "/" << numTotalVerts + 3 + i << " ";
                        faces << numTotalVerts + 2 + i << "/" << numTotalVerts + 2 + i << "/" << numTotalVerts + 2 + i << " ";
                        faces << numTotalVerts + 1 + i << "/" << numTotalVerts + 1 + i << "/" << numTotalVerts + 1 + i << std::endl;
                    }
                    numPolygons++;
                }
                shouldPrint = VERTEX;
                currVertIndex = 0;
            }
        }
        writer << vertices.str() << textureVerts.str() << faces.str();

        std::cout << "Model " << objectNum << ":" << std::endl;
        std::cout << "    # Polygons: " << numPolygons << std::endl;

        shouldPrint = NO_PRINT;
        numPolygons = 0;
        numVertices = 0;
        reader.read(discard, 4);
        reader.read(discard, (16 - reader.tellg() % 16) % 16);
        reader.read(discard, 32);
        objectNum++;
       
    }
    mtlWriter.close();
    writer.close();
    std::cout << objectNum << " models extracted from " << inFileName << "." << std::endl;
    std::cout << totalVertGroups << "lol" << std::endl;
}

void GenHeaderInfo(std::string inFilePath, std::string inFileName, std::vector<ImgSpec>& headerInfo) {
    headerInfo.clear();
    std::string fileName2 = inFileName;
    if (((char)fileName2.at(0) == 'p' || (char)fileName2.at(0) == 'P') && ((char)fileName2.at(1) == 'n' || (char)fileName2.at(1) == 'N')) {
        if ((char)fileName2.at(0) == 'p') {
            fileName2.at(0) = 't';
        }
        else {
            fileName2.at(0) = 'T';
        }
    }
    FindOffset(inFilePath + fileName2, headerInfo);
    std::cout << headerInfo.size() << " images detected." << std::endl;
    for (int i = 0; i < headerInfo.size(); i++) {
        std::cout << "Image " << i << ":" << std::endl;
        std::cout << "    Offset (bytes) in " << fileName2 << ": " << headerInfo.at(i).offset << std::endl;
        std::cout << "    Size: " << headerInfo.at(i).width << "x" << headerInfo.at(i).height << std::endl;
        if (headerInfo.at(i).pixelEncoding == SIXTEEN_BIT) {
            std::cout << "    Palette format: No palette" << std::endl;
        }
        else {
            if (headerInfo.at(i).smallColors) {
                std::cout << "    Palette format: 16-bit" << std::endl;
            }
            else {
                std::cout << "    Palette format: 24-bit" << std::endl;
            }
        }

        if (headerInfo.at(i).pixelEncoding == FOUR_BIT) {
            std::cout << "    Pixel encoding: 4-bit (Note: 4-bit image conversion not yet implemented)" << std::endl;
        }
        else if (headerInfo.at(i).pixelEncoding == EIGHT_BIT) {
            std::cout << "    Pixel encoding: 8-bit" << std::endl;
        }
        else {
            std::cout << "    Pixel encoding: 16-bit" << std::endl;
        }
    }
}