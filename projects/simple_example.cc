#include <string>
#include <iostream>
#include <iomanip>
#include <fstream>
#include "rocksdb/db.h"
#include "rocksdb/slice.h"
#include "rocksdb/cache.h"
#include "rocksdb/options.h"
#include "rocksdb/advanced_options.h"
#include "rocksdb/table.h"

using namespace rocksdb;
std::string kDBPath = "/tmp/cs561_project1";

inline void showProgress(const uint64_t& workload_size, const uint64_t& counter) {

    if (counter / (workload_size / 100) >= 1) {
        for (int i = 0; i < 104; i++) {
            std::cout << "\b";
            fflush(stdout);
        }
    }
    for (int i = 0; i < counter / (workload_size / 100); i++) {
        std::cout << "=";
        fflush(stdout);
    }
    std::cout << std::setfill(' ') << std::setw(101 - counter / (workload_size / 100));
    std::cout << counter * 100 / workload_size << "%";
    fflush(stdout);

    if (counter == workload_size) {
        std::cout << "\n";
        return;
    }
}

void runWorkload(Options& op, WriteOptions& write_op, ReadOptions& read_op) {
    DB* db;

    op.create_if_missing = true;
    op.write_buffer_size = 8 * 1024 * 1024;

    {
        op.memtable_factory = std::shared_ptr<VectorRepFactory>(new VectorRepFactory);
        op.allow_concurrent_memtable_write = false;
    }

    {
        //op.memtable_factory = std::shared_ptr<SkipListFactory>(new SkipListFactory);
    }

    {
        //op.memtable_factory = std::shared_ptr<MemTableRepFactory>(NewHashSkipListRepFactory());
        //op.allow_concurrent_memtable_write = false;
    }

    {
        //op.memtable_factory = std::shared_ptr<MemTableRepFactory>(NewHashLinkListRepFactory());
        //op.allow_concurrent_memtable_write = false;
    }


    //BlockBasedTableOptions table_options;
    //table_options.block_cache = NewLRUCache(8*1048576);
    //op.table_factory.reset(NewBlockBasedTableFactory(table_options));

    Status s = DB::Open(op, kDBPath, &db);
    if (!s.ok()) std::cerr << s.ToString() << std::endl;
    assert(s.ok());

    // opening workload file for the first time
    std::ifstream workload_file;
    workload_file.open("workload.txt");
    assert(workload_file);
    // doing a first pass to get the workload size
    uint64_t workload_size = 0;
    std::string line;
    while (std::getline(workload_file, line))
        ++workload_size;
    workload_file.close();

    workload_file.open("workload.txt");
    assert(workload_file);

    Iterator* it = db->NewIterator(read_op); // for range reads
    uint64_t counter = 0; // for progress bar

    while (!workload_file.eof()) {
        char instruction;
        long key, start_key, end_key;
        std::string value;
        workload_file >> instruction;
        switch (instruction)
        {
        case 'I': // insert
            workload_file >> key >> value;
            // Put key-value
            s = db->Put(write_op, std::to_string(key), value);
            if (!s.ok()) std::cerr << s.ToString() << std::endl;
            assert(s.ok());
            counter++;
            break;

        case 'Q': // probe: point query
            workload_file >> key;
            s = db->Get(read_op, std::to_string(key), &value);
            //if (!s.ok()) std::cerr << s.ToString() << "key = " << key << std::endl;
            // assert(s.ok());
            counter++;
            break;

        case 'S': // scan: range query
            workload_file >> start_key >> end_key;
            it->Refresh();
            assert(it->status().ok());
            for (it->Seek(std::to_string(start_key)); it->Valid(); it->Next()) {
                //std::cout << "found key = " << it->key().ToString() << std::endl;
                if (it->key().ToString() == std::to_string(end_key)) {
                    break;
                }
            }
            if (!it->status().ok()) {
                std::cerr << it->status().ToString() << std::endl;
            }
            counter++;
            break;

        default:
            std::cerr << "ERROR: Case match NOT found !!" << std::endl;
            break;
        }

        if (workload_size < 100) workload_size = 100;
        if (counter % (workload_size / 100) == 0) {
            showProgress(workload_size, counter);
        }
    }


    workload_file.close();
    s = db->Close();
    if (!s.ok()) std::cerr << s.ToString() << std::endl;
    assert(s.ok());
    delete db;

    std::cout << "\n----------------------Closing DB-----------------------" << std::endl;

    return;
}

int main() {
    Options options;
    WriteOptions write_op;
    ReadOptions read_op;
    runWorkload(options, write_op, read_op);
}