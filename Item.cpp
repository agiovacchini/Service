//
//  Item.cpp
//  PromoCalculator
//
//  Created by Andrea Giovacchini on 04/01/14.
//  Copyright (c) 2014 Andrea Giovacchini. All rights reserved.
//

#include "Item.h"

Item::Item()
{
	this->code = 0 ;
	this->price = 0 ;
	this->description = "" ;
	this->departmentCode = 0 ;
}

Item::Item(uint64_t pCode, long pPrice, string pDescription, uint64_t pDepartmentCode )
{
    this->code = pCode ;
    this->price = pPrice ;
	this->description = pDescription ;
	this->departmentCode = pDepartmentCode ;
}

void Item::setCode( uint64_t pCode ) {
    this->code = pCode ;
}

uint64_t Item::getCode() const {
    return this->code ;
}

void Item::setPrice( long pPrice ) {
    this->price = pPrice ;
}

long Item::getPrice() const {
    return this->price ;
}

void Item::setDescription( string pDescription ) {
    this->description = pDescription ;
}

string Item::getDescription() const {
    return this->description ;
}

//E' una referenza all'oggetto reparto
void Item::setDepartmentCode(uint64_t pDepartmentCode) {
	this->departmentCode = pDepartmentCode;
}

uint64_t Item::getDepartmentCode() {
    return this->departmentCode ;
}

void Item::setLinkedBarCode( uint64_t pLinkedBarCode ) {
    this->linkedBarCode = pLinkedBarCode ;
}

uint64_t Item::getLinkedBarCode() const {
    return this->linkedBarCode ;
}

string Item::toStr() const {
	std::stringstream tempStringStream;
	tempStringStream.str(std::string());
	tempStringStream.clear();
	tempStringStream << this->code
		<< ",\"" << this->description << "\""
        << "," << this->departmentCode
        << "," << this->price
        << "," << this->linkedBarCode
    ;

	return tempStringStream.str();
}