//
//  ArchiveMap.h
//  PromoCalculator
//
//  Created by Andrea Giovacchini on 18/03/14.
//  Copyright (c) 2014 Andrea Giovacchini. All rights reserved.
//

#ifndef PromoCalculator_ArchiveMap_h
#define PromoCalculator_ArchiveMap_h

#include <iostream>
#include <string>
#include <sstream>

template <class T>
class ArchiveMap {
    std::map <unsigned long long, T> elementsMap ;
public:
    void addElement(T& elem) {elementsMap[elem.getCode()]=elem;};
    
    std::string getElementStr(unsigned long long key) {return elementsMap[key].toStr();} ;
    
    bool dumpToFile(std::string pFileName)
    {
        std::ofstream outFile( pFileName );
        for(typename std::map<unsigned long long, T>::iterator it = elementsMap.begin(); it != elementsMap.end(); it++) {
            outFile << it->second.toStr() << std::endl ;
        }        
        outFile.close() ;
        
        return true;
    }
    
    T& operator[](unsigned long long key) { return
        elementsMap[key] ; } ;
    
    
};

#endif
