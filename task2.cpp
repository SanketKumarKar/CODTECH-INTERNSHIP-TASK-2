#include <iostream>
#include <fstream>
#include <string>
#include <vector>
#include <thread>
#include <chrono>
#include <zlib.h>
#include <stdexcept>

using namespace std;

// NOTE: To compile this code, you need to have zlib installed.
// On Debian/Ubuntu: sudo apt-get install zlib1g-dev
// On Fedora/CentOS: sudo yum install zlib-devel
// On macOS (with Homebrew): brew install zlib
//
// Compile command: g++ task2.cpp -o task2 -std=c++11 -pthread -lz

const int CHUNK_SIZE = 1024 * 1024; // 1MB chunks

// Function to compress a chunk of data
vector<char> compress_chunk(const vector<char>& data) {
    uLongf compressed_size = compressBound(data.size());
    vector<char> compressed_data(compressed_size);
    if (compress((Bytef*)compressed_data.data(), &compressed_size, (const Bytef*)data.data(), data.size()) != Z_OK) {
        throw runtime_error("Compression failed");
    }
    compressed_data.resize(compressed_size);
    return compressed_data;
}

// Function to decompress a chunk of data
vector<char> decompress_chunk(const vector<char>& compressed_data, uLong original_size) {
    vector<char> decompressed_data(original_size);
    uLongf decompressed_size = original_size;
    if (uncompress((Bytef*)decompressed_data.data(), &decompressed_size, (const Bytef*)compressed_data.data(), compressed_data.size()) != Z_OK) {
        throw runtime_error("Decompression failed");
    }
    decompressed_data.resize(decompressed_size);
    return decompressed_data;
}

// Compresses a file using multiple threads
void compress_file_multithreaded(const string& input_filename, const string& output_filename) {
    ifstream in_file(input_filename, ios::binary);
    if (!in_file) {
        cerr << "Error: Cannot open input file " << input_filename << endl;
        return;
    }

    ofstream out_file(output_filename, ios::binary);
    if (!out_file) {
        cerr << "Error: Cannot open output file " << output_filename << endl;
        return;
    }

    auto start_time = chrono::high_resolution_clock::now();

    vector<thread> threads;
    vector<vector<char>> chunks;
    vector<vector<char>> compressed_chunks;

    // Read file into chunks
    while (in_file) {
        vector<char> chunk(CHUNK_SIZE);
        in_file.read(chunk.data(), CHUNK_SIZE);
        chunk.resize(in_file.gcount());
        if (!chunk.empty()) {
            chunks.push_back(chunk);
        }
    }
    
    compressed_chunks.resize(chunks.size());

    // Threading logic: each thread compresses one chunk
    for (size_t i = 0; i < chunks.size(); ++i) {
        threads.emplace_back([&, i]() {
            compressed_chunks[i] = compress_chunk(chunks[i]);
        });
    }

    for (auto& t : threads) {
        t.join();
    }

    // Write compressed chunks to file
    for (const auto& compressed_chunk : compressed_chunks) {
        uint32_t chunk_size = compressed_chunk.size();
        uint32_t original_size = chunks[&compressed_chunk - &compressed_chunks[0]].size();
        out_file.write(reinterpret_cast<const char*>(&original_size), sizeof(original_size));
        out_file.write(reinterpret_cast<const char*>(&chunk_size), sizeof(chunk_size));
        out_file.write(compressed_chunk.data(), chunk_size);
    }

    auto end_time = chrono::high_resolution_clock::now();
    chrono::duration<double> elapsed = end_time - start_time;
    cout << "Compression finished in " << elapsed.count() << " seconds." << endl;
}

// Decompresses a file using multiple threads
void decompress_file_multithreaded(const string& input_filename, const string& output_filename) {
    ifstream in_file(input_filename, ios::binary);
    if (!in_file) {
        cerr << "Error: Cannot open input file " << input_filename << endl;
        return;
    }

    ofstream out_file(output_filename, ios::binary);
    if (!out_file) {
        cerr << "Error: Cannot open output file " << output_filename << endl;
        return;
    }

    auto start_time = chrono::high_resolution_clock::now();

    vector<thread> threads;
    vector<vector<char>> compressed_chunks;
    vector<uLong> original_sizes;
    vector<vector<char>> decompressed_chunks;

    // Read compressed chunks from file
    while (in_file) {
        uint32_t original_size, chunk_size;
        in_file.read(reinterpret_cast<char*>(&original_size), sizeof(original_size));
        in_file.read(reinterpret_cast<char*>(&chunk_size), sizeof(chunk_size));
        if (in_file.gcount() == 0) break;

        vector<char> compressed_chunk(chunk_size);
        in_file.read(compressed_chunk.data(), chunk_size);
        
        compressed_chunks.push_back(compressed_chunk);
        original_sizes.push_back(original_size);
    }

    decompressed_chunks.resize(compressed_chunks.size());

    // Threading logic: each thread decompresses one chunk
    for (size_t i = 0; i < compressed_chunks.size(); ++i) {
        threads.emplace_back([&, i]() {
            decompressed_chunks[i] = decompress_chunk(compressed_chunks[i], original_sizes[i]);
        });
    }

    for (auto& t : threads) {
        t.join();
    }

    // Write decompressed chunks to file
    for (const auto& chunk : decompressed_chunks) {
        out_file.write(chunk.data(), chunk.size());
    }

    auto end_time = chrono::high_resolution_clock::now();
    chrono::duration<double> elapsed = end_time - start_time;
    cout << "Decompression finished in " << elapsed.count() << " seconds." << endl;
}


int main(int argc, char* argv[]) {
    if (argc != 4) {
        cerr << "Usage: " << argv[0] << " <compress|decompress> <input_file> <output_file>" << endl;
        return 1;
    }

    string mode = argv[1];
    string input_file = argv[2];
    string output_file = argv[3];

    if (mode == "compress") {
        compress_file_multithreaded(input_file, output_file);
    } else if (mode == "decompress") {
        decompress_file_multithreaded(input_file, output_file);
    } else {
        cerr << "Invalid mode. Use 'compress' or 'decompress'." << endl;
        return 1;
    }

    return 0;
}
