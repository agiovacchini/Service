//
//  Barcodes.cpp
//  PromoCalculator
//
//  Created by Andrea Giovacchini on 08/01/14.
//  Copyright (c) 2014 Andrea Giovacchini. All rights reserved.
//

#include "Barcodes.h"
#include "BaseTypes.h"

void Barcodes::setCode( uint64_t pCode ) {
    this->code = pCode ;
}

uint64_t Barcodes::getCode() const {
    return this->code ;
}

void Barcodes::setItemCode( uint64_t pItemCode ) {
    this->itemCode = pItemCode ;
}

uint64_t Barcodes::getItemCode() const {
    return this->itemCode ;
}

string Barcodes::toStr() const {
    std::stringstream row ;
    
    row << this->code
    << "," << this->itemCode ;
    
    return row.str() ;
}