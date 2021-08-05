# NightsImageTool
A tool I threw together to extract images from Steam NiGHTS Into Dreams and mod them as well. I'm no expert at making programs work on different machines but this _should_ work if you have Windows 10 64-bit. It probably won't work on a Mac. Here are the different functions:

Output: Enter the name of a game file and the program will search for an image header and use it to extract all image data from the file. Each image will be saved to the current home directory. I haven't figured out how to unscramble certain types of images (those with 4 bits per pixel) so those will be a mess of pixels, but the others should work.

Input: Enter the name of a game file to modify. The program will output a list of image data detected in the file, with each one having a number and a width x height in pixels. Enter the index of the image you wish to modify, then the name of a .bmp image file in the home directory. (4-bit images still cannot be modified, but many of them seem to be unused anyway) The program will only accept a 24-bit bitmap with the same dimensions as the original image to avoid corruption. The image should also have a maximum of 256 colors, as this is the size of the in-game palette. If the uploaded image has more than 256 colors, any additional colors will be treated as having color #1. **Make sure to back up or copy the game files before using this function, as the change is permanent.**

Configure: Enter a home directory and game directory for the program to use. The home directory is where extracted images will be saved and where modified textures can be uploaded from. The game directory is simply where the program will look for NiGHTS Into Dreams files. These will be saved for next time.

Gallery: Outputs all 153 gallery files at once, to save time.
