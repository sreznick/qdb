#include <iostream>
#include <chrono>
#include <fcntl.h>
#include <string>
#include <vector>
#include <algorithm>
#include <cmath>
#include <unistd.h>
#include <fstream>
#include <random>

int main(void) {
	const int FILE_SIZE = 100000000;
	std::size_t size = 1000 * sizeof(char);
	std::random_device rd; 
    std::mt19937 gen(rd()); 
    std::uniform_int_distribution<> dis(0, FILE_SIZE); //100 MB

    std::ofstream file("random_data.bin", std::ios::binary);
    for (int n = 0; n < FILE_SIZE; ++n){
        char random_char = dis(gen);
        file.write(&random_char, sizeof(random_char));
    }

    file.close();
	std::vector<float> writes, reads, delays;

	// Generating random offsets
	const int N = 10000;
    int offsets[N];
    for (int i = 0; i < N; i++) {
        offsets[i] = rand() % FILE_SIZE;
    }
    
    // Measure delay time
    float write_speed = 0, read_speed = 0, delay = 0;
	float write_dispersion = 0, read_dispersion = 0, delay_dispersion = 0;

	for (std::size_t i = 0; i < N; i++) {
		char* data = (char*)malloc(size);
        int handle = open("random_data.bin", O_RDWR | O_CREAT | O_TRUNC, 0644);
		std::chrono::high_resolution_clock::time_point begin, end;
		
		begin = std::chrono::high_resolution_clock::now();
		lseek(handle, offsets[i], SEEK_SET);
		end = std::chrono::high_resolution_clock::now();
		if (read(handle, data, size) == -1) {
			std::cout << "Error reading the file" << std::endl;
		} else {
			delays.push_back(std::chrono::duration_cast<std::chrono::duration<float>>(end - begin).count());
		}

		begin = std::chrono::high_resolution_clock::now();
		if (write(handle, data, size) == -1) {
			std::cout << "Error writing the file" << std::endl;
		} else {
			fsync(handle);
			end = std::chrono::high_resolution_clock::now();
			writes.push_back(std::chrono::duration_cast<std::chrono::duration<float>>(end - begin).count());	
		}
		begin = std::chrono::high_resolution_clock::now();

		if(read(handle, data, size) == -1) {
			std::cout << "Error reading the file" << std::endl;
		} else {
			end = std::chrono::high_resolution_clock::now();
			close(handle);
			reads.push_back(std::chrono::duration_cast<std::chrono::duration<float>>(end - begin).count());
		}
	}
	
	std::size_t w_away = writes.size() * 0.05;
	sort(writes.begin(), writes.end());
	for (std::size_t i = w_away; i <  writes.size() - w_away; i++) {
		write_speed += writes[i];
	}

	std::size_t r_away = reads.size() * 0.05;
	sort(reads.begin(), reads.end());
	for (std::size_t i = r_away; i <  reads.size() - r_away; i++) {
		read_speed += reads[i];
	}

	std::size_t d_away = delays.size() * 0.05;
	sort(delays.begin(), delays.end());
	for (std::size_t i = d_away; i <  delays.size() - d_away; i++) {
		delay += delays[i];
	}

	write_speed = size / (write_speed / (writes.size() - 2 * w_away));
	read_speed = size / (read_speed / (reads.size() - 2 * r_away));
	delay = delay / (delays.size() - 2 * d_away);

	for (std::size_t i = w_away; i < writes.size() - w_away; i++) {
		write_dispersion += (size / writes[i]) * (size / writes[i]);
	}
	for (std::size_t i = r_away; i < reads.size() - r_away; i++) {
		read_dispersion += (size / reads[i]) * (size / reads[i]);
	}
	for (std::size_t i = d_away; i < delays.size() - d_away; i++) {
		delay_dispersion += delays[i] * delays[i];
	}

	write_dispersion =  write_dispersion / (writes.size() - 2 * w_away) - write_speed * write_speed;
	read_dispersion =  read_dispersion / (reads.size() - 2 * r_away) - read_speed * read_speed;
	delay_dispersion =  delay_dispersion / (delays.size() - 2 * d_away) - delay * delay;

	std::cout << "\n" << "\n" << "\n"
		 << "Write (Mb/s): " << write_speed / 8000 << "   " << "Write dispersion: " << write_dispersion << "\n" 
		 << "Read (Mb/s): " << read_speed / 8000 << "   " << "Read dispersion: " << read_dispersion << "\n" 
		 << "Delay (ns): " << delay * 1000 << "   " << "Delay dispersion: " << delay_dispersion << "\n"
		 << "\n" << "\n" << "\n";
	return 0;
}