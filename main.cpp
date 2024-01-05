#include <iostream>
#include <chrono>
#include <fstream>
#include <cstring>
#include <algorithm>
#include <map>


const int HEIGHT = 512;
const int WIDTH = 512;


void perform_operation(unsigned char* input_image, unsigned char* output_image, int ksize);

int main(int argc, char** argv)
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

    // Loop over pixels in image
    for (int p = 0; p < HEIGHT * WIDTH; p++)
    {
        // Coordinates for current pixel
        const int p_x = p % WIDTH;
        const int p_y = p / HEIGHT; // floor(i / HEIGHT) (integer division)

        std::map<unsigned char, int> count_red{};
        std::map<unsigned char, int> count_green{};
        std::map<unsigned char, int> count_blue{};

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
                else
                {
                    // Get pixel values and increment count for found values
                    const unsigned char red = input_image[get_index(x, y, 0)];
                    const unsigned char green = input_image[get_index(x, y, 1)];
                    const unsigned char blue = input_image[get_index(x, y, 2)];
                    count_red[red]++;
                    count_green[green]++;
                    count_blue[blue]++;
                }
            }
        }

        // Set current pixel to most common value in R, separate for each color channel
        const auto compare_value = [](const std::pair<unsigned char, int>& a, const std::pair<unsigned char, int>& b) { return a.second < b.second; };
        const auto max_count_red = std::max_element(count_red.begin(), count_red.end(), compare_value);
        const auto max_count_green = std::max_element(count_green.begin(), count_green.end(), compare_value);
        const auto max_count_blue = std::max_element(count_blue.begin(), count_blue.end(), compare_value);
        output_image[get_index(p_x, p_y, 0)] = max_count_red->first;
        output_image[get_index(p_x, p_y, 1)] = max_count_green->first;
        output_image[get_index(p_x, p_y, 2)] = max_count_blue->first;
    }
}
