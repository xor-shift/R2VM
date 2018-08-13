#include "VMManager.h"

VMManager::VMManager(){

}

VMManager::~VMManager(){

}

void VMManager::loadROM(std::string filename, bool binary){
    std::array<uint32_t, 65536> tempr;
    if(binary){
        std::ifstream binaryfilestream(filename, std::ios::binary);
        if(!binaryfilestream){
            std::cout<<"[error] Failed to open "<<filename<<std::endl;
            return;
        }
        std::cout<<"[info] Reading from "<<filename<<std::endl;
        uint32_t temp;
        uint16_t dp = 0;
        while(binaryfilestream.read(reinterpret_cast<char*>(&temp), sizeof(temp))){
            tempr[dp] = temp;
            dp++;
            if(!dp){
                std::cout<<"[warning] dp got carried while reading from file, will continue from 0"<<std::endl;
            }
        }
        std::cout<<"[info] Done reading from "<<filename<<std::endl;
        binaryfilestream.close();
    }else{
        std::ifstream dumpfilestream(filename);
        if(!dumpfilestream){
            std::cout<<"[error] Failed to open "<<filename<<std::endl;
            return;
        }
        std::cout<<"[info] Reading from "<<filename<<std::endl;
        std::string tempstring;
        std::vector<std::string> filecontents;
        while(std::getline(dumpfilestream, tempstring))
            filecontents.push_back(tempstring);
        dumpfilestream.close();
        std::cout<<"[info] Done reading from "<<filename<<std::endl;
        std::cout<<"[info] Converting file contents to binary"<<std::endl;
        uint16_t dp = 0;
        for(auto &line : filecontents){
            try{
                tempr[dp] = std::stoul(line, nullptr, 16);
            }catch(std::invalid_argument *inval){
                std::cout<<"[Warning] Invalid line in dump file!"<<std::endl;
                std::cout<<"[Debug] "<<inval->what()<<std::endl;
                continue;
            }
            dp++;
        }
        std::cout<<"[info] Done converting file contents"<<std::endl;
    }
    loaded = true;
    load(tempr);
}

void VMManager::_dump(bool core){
    dump(core);
}

void VMManager::run(bool stepmode){
    if(!loaded)
        return;
    std::chrono::time_point<std::chrono::system_clock> t1 = std::chrono::system_clock::now();
    std::chrono::time_point<std::chrono::system_clock> t2;
    uint64_t steps = 0;
    if(!stepmode){
        while(!halt){
            if(skiptick){
                skiptick--;
                continue;
            }
            step();
            steps++;
            if(halt){
                t2 = std::chrono::system_clock::now();
                char temp;
                std::cout<<"[Info] An HLT was caught, press 'c' to continue, anything else to end program."<<std::endl;
                std::cin>>temp;
                if(temp == 'c'){
                    halt=false;
                    continue;
                }
                auto millis = std::chrono::duration_cast<std::chrono::milliseconds>(t2-t1);
                std::cout<<"[info] Program took: "<<millis.count()<<"ms to run"<<std::endl;
                std::cout<<"[info] Steps taken: "<<steps<<std::endl;
                if(millis.count())
                    std::cout<<"[info] Average speed was: "<<(steps/(millis.count()))<<"KHz"<<std::endl;
                else
                    std::cout<<"[info] Program ended too quickly, no average speed can be calculated."<<std::endl;
            }
        }
    }else{
        char temp;
        while(!halt){
            dump();
            step();
            std::cin>>temp;
            if(halt){
                t2 = std::chrono::system_clock::now();
                std::cout<<"[Info] An HLT was caught, press 'c' to continue, anything else to end program."<<std::endl;
                std::cin>>temp;
                if(temp == 'c'){
                    halt=false;
                    continue;
                }
            }
        }
    }
}
