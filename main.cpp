#include <iostream>
#include <chrono>
#include <fstream>
#include <cstring>
#include <algorithm>
#include <array>


const int HEIGHT = 512;
const int WIDTH = 512;


void perform_operation(unsigned char* input_image, unsigned char* output_image, int ksize);

int main()
{
    unsigned char input_image[HEIGHT*WIDTH*3];
    unsigned char output_image[HEIGHT*WIDTH*3];

    std::fstream fin("baboon_buf.dat", std::ios::in | std::ios::binary);
    fin.read((char*)input_image, HEIGHT*WIDTH*3);

    auto start = std::chrono::high_resolution_clock::now();
    perform_operation(input_image, output_image, 23);
    auto end = std::chrono::high_resolution_clock::now();

    std::fstream fout("baboon_processed.dat", std::ios::out | std::ios::binary);
    fout.write((char*)output_image, HEIGHT*WIDTH*3);

    auto duration = std::chrono::duration_cast<std::chrono::microseconds>(end - start);
    std::cout << duration.count() << std::endl;

    return 0;
}

int get_index(int x, int y, int ch)
{
    int index = HEIGHT*WIDTH*ch + y*WIDTH + x;
    return index;
}


void perform_operation(unsigned char* input_image, unsigned char* output_image, int ksize)
{
    memcpy(output_image, input_image, HEIGHT*WIDTH*3);

    /* For each pixel p in the image, consider a square region R around the pixel with sides 2*ksize + 1:
        Find the most frequent pixel value v in R (including p).
        Set the value of the pixel to v.
        If multiple values are just as common, default to the lowest value of these.
            Example: if R is [(1, 1, 2), (3, 3, 3), (1, 2, 6)], then v should be 1.
        Pixels outside the image should be considered to have a value of 0.
        Each color channel in the RGB image should be processed independently. 
    */
    
    // Pixel values. Index is color value, value at index is the count
    const int MAX_PIXEL_VALUE = 255 + 1;
    std::array<int, MAX_PIXEL_VALUE> count_red;
    std::array<int, MAX_PIXEL_VALUE> count_green;
    std::array<int, MAX_PIXEL_VALUE> count_blue;

    // Loop over pixels in image
    for (int p = 0; p < HEIGHT * WIDTH; p++)
    {
        // Coordinates for current pixel
        const int p_x = p % WIDTH;
        const int p_y = p / HEIGHT; // floor(i / HEIGHT) (integer division)

        count_red.fill(0);
        count_green.fill(0);
        count_blue.fill(0);

        // Loop over region R around pixel p
        for (int i = -ksize; i <= ksize; i++)
        {
            for (int j = -ksize; j <= ksize; j++)
            {
                const int x = p_x + i;
                const int y = p_y + j;
                
                // Count pixel as black if out of bounds
                if (x < 0 or x >= WIDTH or y < 0 or y >= HEIGHT)
                {
                    count_red[0]++;
                    count_green[0]++;
                    count_blue[0]++;
                }
                // Get pixel values and increment count for found values
                else
                {
                    const unsigned char red = input_image[get_index(x, y, 0)];
                    const unsigned char green = input_image[get_index(x, y, 1)];
                    const unsigned char blue = input_image[get_index(x, y, 2)];
                    count_red[red]++;
                    count_green[green]++;
                    count_blue[blue]++;
                    if (red >= MAX_PIXEL_VALUE or green >= MAX_PIXEL_VALUE or blue >= MAX_PIXEL_VALUE)
                        std::cout << "\n!!!CRASHERINO PROGRAM NOT WORKING DESTROY COMPUTER NOW!!!\n\n";
                }
            }
        }

        // Set current pixel to most common value in R, separate for each color channel
        int max_red_count{}, max_green_count{}, max_blue_count{};
        unsigned char max_red_value{}, max_green_value{}, max_blue_value{}; // Array indices

        for (int i = 0; i < MAX_PIXEL_VALUE; i++)
        {
            int curr_red_count = count_red[i];
            if (curr_red_count > max_red_count)
            {
                max_red_count = curr_red_count;
                max_red_value = i;
            }
            
            int curr_green_count = count_green[i];
            if (curr_green_count > max_green_count)
            {
                max_green_count = curr_green_count;
                max_green_value = i;
            }
            
            int curr_blue_count = count_blue[i];
            if (curr_blue_count > max_blue_count)
            {
                max_blue_count = curr_blue_count;
                max_blue_value = i;
            }
        }
        output_image[get_index(p_x, p_y, 0)] = max_red_value;
        output_image[get_index(p_x, p_y, 1)] = max_green_value;
        output_image[get_index(p_x, p_y, 2)] = max_blue_value;
    }
}
