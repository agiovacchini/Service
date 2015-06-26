//
//  BaseTypes.cpp
//  PromoCalculator
//
//  Created by Andrea Giovacchini on 03/03/14.
//  Copyright (c) 2014 Andrea Giovacchini. All rights reserved.
//

#include "BaseTypes.h"

bool fileMove(std::string pFileOri, std::string pFileDest)
{
    boost::filesystem::path src(pFileOri);
    boost::filesystem::path dest(pFileDest);
    
    try {
        boost::filesystem::rename(src, dest);
    }
    catch (std::exception const& e)
    {
        return false;
    }
    return boost::filesystem::exists(dest);
}

bool fileDelete(std::string pFileName)
{
    boost::filesystem::path fileToDelete(pFileName);
    
    try {
        if (boost::filesystem::exists(fileToDelete))
        {
            boost::filesystem::remove(fileToDelete);
        }
    }
    catch (std::exception const& e)
    {
        return false;
    }
    return !boost::filesystem::exists(fileToDelete);
}
