//
//  Department.cpp
//  PromoCalculator
//
//  Created by Andrea Giovacchini on 05/01/14.
//  Copyright (c) 2014 Andrea Giovacchini. All rights reserved.
//

#include "Department.h"


Department::Department()
{
	this->code = 0;
    this->parentCode = 0;
	this->description = "";
}

Department::Department(uint64_t pCode, uint64_t pParentCode, string pDescription)
{
	this->code = pCode;
	this->parentCode = pParentCode;
    this->description = pDescription;
}

void Department::setCode( uint64_t pCode ) {
    this->code = pCode ;
}

uint64_t Department::getCode() const {
    return this->code ;
}

void Department::setParentCode( uint64_t pParentCode ) {
    this->parentCode = pParentCode ;
}

uint64_t Department::getParentCode() const {
    return this->parentCode ;
}

void Department::setDescription( string pDescription ) {
    this->description = pDescription ;
}

string Department::getDescription() const {
    return this->description ;
}

string Department::toStr() const {
	std::stringstream tempStringStream;
    tempStringStream.str(std::string());
    tempStringStream.clear();
    try {
        tempStringStream << this->code
        << "," << this->parentCode
		<< ",\"" << this->description << "\"" ;
    } catch (std::exception const& e)
    {
        std::cout << "Dept: " << e.what() << std::endl ;
    }


	return tempStringStream.str();
}
