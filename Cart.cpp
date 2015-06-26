//
//  Cart.cpp
//  PromoCalculator
//
//  Created by Andrea Giovacchini on 04/01/14.
//  Copyright (c) 2014 Andrea Giovacchini. All rights reserved.
//

#include "BaseTypes.h"
#include "Cart.h"

#include <sstream>
#include <stdio.h>
#include <chrono>
#include <ctime>
#include <locale>
#include <fstream>
#include <mutex>
//#include <pcap.h>
#include "boost/format.hpp"
#include "boost/filesystem.hpp"
#include "boost/date_time/posix_time/posix_time.hpp"
#include <boost/date_time/posix_time/posix_time_io.hpp>
//static string basePath = "./" ;

using namespace std;

std::mutex cart_sendtopos_mutex ;

Cart::Cart( string pBasePath, uint32_t pNumber, unsigned int pAction, bool pDummyRCS )
{
    std::stringstream tempStringStream;
    std::ofstream tmpTransactionFile;
    number = pNumber ;
    itemsNumber = 0 ;
    nextRequestId = 1 ;
    loyCardsNumber = 0 ;
    basePath = pBasePath ;
    dummyRCS = pDummyRCS ;
    
    cartItemsMap.clear() ;
    itemsLocalCopyMap.clear() ;
    
    CartRow totalCartRow = { TOTAL, 0 } ;
    
    totalsMap[0].totalAmount = 0 ;
    totalsMap[0].itemsNumber = 0 ;
    cartItemsMap[&totalsMap[0]] = totalCartRow ;
    cartFileName = (boost::format("%sCARTS/%010lu.cart") % basePath % number).str() ;
    tmpTransactionFileName = (boost::format("%sCARTS/%010lu.transaction_in_progress") % basePath % number).str() ;
    tmpTransactionFile.open( tmpTransactionFileName, fstream::app );
    tmpTransactionFile.close() ;

    BOOST_LOG_SEV(my_logger_ca, lt::info) << "- CART " << this->number << " - " << cartFileName << " - " << tmpTransactionFileName ;
    
    switch (pAction)
    {
        case GEN_CART_NEW:
            state = CART_STATE_READY_FOR_ITEM ;
            break ;
        case GEN_CART_LOAD:
            state = CART_STATE_TMPFILE_LOADING ;
            break;
        default:
            break;
    }
}


void Cart::setNumber( uint32_t pNumber )
{
    this->number = pNumber ;
}

uint32_t Cart::getNumber() const
{
    return this->number ;
}

long Cart::getState() const {
    return this->state;
}

void Cart::setState( unsigned int pState ) {
    //BOOST_LOG_SEV(my_logger_ca, lt::info) << "Carrello: passaggio di stato da " << this->state << " a " << pState ;
    this->state = pState ;
}

uint32_t Cart::getRequestId()
{
    return this->nextRequestId ;
}

uint32_t Cart::getNextRequestId()
{
    this->nextRequestId++ ;
    return this->nextRequestId ;
}

unsigned int Cart::getLoyCardsNumber() const
{
    return this->loyCardsNumber ;
}
/*Totals Cart::addItemByBarcode(uint64_t pBarcode)
 {
 //bs.itemsMap[6945339]
 }*/

void Cart::writeTransactionRow( string row )
{
    std::ofstream tmpTransactionFile;
    tmpTransactionFile.open( tmpTransactionFileName, fstream::app );
	tmpTransactionFile << boost::posix_time::to_iso_string(boost::posix_time::microsec_clock::universal_time()) << "," << row << "\n";
    tmpTransactionFile.close() ;
}

long Cart::addLoyCard( uint64_t pLoyCardNumber, unsigned int maxCardNumber )
{
    if ( (this->getState()==CART_STATE_TMPFILE_LOADING) || (this->getState()==CART_STATE_READY_FOR_ITEM) )
    {
        if (this->loyCardsNumber < maxCardNumber )
        {
            typedef std::map<unsigned int, uint64_t>::iterator it_type;
            std::stringstream tempStringStream;
            for(it_type iterator = loyCardsMap.begin(); iterator != loyCardsMap.end(); iterator++) {
                if (iterator->second==pLoyCardNumber)
                {
                    return RC_LOY_CARD_ALREADY_PRESENT ;
                }
            }
            loyCardsMap[loyCardsNumber] = pLoyCardNumber ;
            if (this->getState()==CART_STATE_READY_FOR_ITEM)
            {
                tempStringStream.str(std::string());
                tempStringStream.clear();
                tempStringStream << "L,A," << pLoyCardNumber << "," << "1" ;
                this->writeTransactionRow(tempStringStream.str() );
            }
            this->loyCardsNumber++ ;
            return RC_OK ;
            
        } else {
            return RC_LOY_MAX_CARD_NUMBER ;
            //respStringStream << "{Numero massimo di carte fedelta superato}" ;
        }
        return RC_OK ;
    } else {
        return this->state ;
    }
}

long Cart::removeLoyCard( uint64_t pLoyCardNumber )
{
    if ( (this->getState()==CART_STATE_TMPFILE_LOADING) || (this->getState()==CART_STATE_READY_FOR_ITEM) )
    {
        typedef std::map<unsigned int, uint64_t>::iterator itLoyCards;
        std::stringstream tempStringStream;
        for(itLoyCards iterator = loyCardsMap.begin(); iterator != loyCardsMap.end(); iterator++) {
            if (iterator->second==pLoyCardNumber)
            {
                loyCardsMap.erase(iterator) ;
                if (this->getState()==CART_STATE_READY_FOR_ITEM)
                {
                    tempStringStream.str(std::string());
                    tempStringStream.clear();
                    tempStringStream << "L,V," << pLoyCardNumber << "," << "1" ;
                    this->writeTransactionRow(tempStringStream.str() );
                }
                this->loyCardsNumber-- ;
                return RC_OK ;
            }
        }
        return RC_LOY_CARD_NOT_PRESENT ;
    } else {
        return this->state ;
    }
}

bool Cart::updateLocalItemMap(Item& pItem, Department& pDept)
{
    std::stringstream tempStringStream;
	//BOOST_LOG_SEV(my_logger_ca, lt::debug) << "- CA - Brucia all'inferno updateLocalItemMap - " << pItem.getDepartment().getCode();

    if ((itemsLocalCopyMap.find(pItem.getCode()) != itemsLocalCopyMap.end()))
    {
        //std::cout << "f" << std::endl ;
        return false ;
    } else {
        if (this->getState()==CART_STATE_READY_FOR_ITEM)
        {
            tempStringStream.str(std::string());
            tempStringStream.clear();
            tempStringStream << "K,D," << pDept.toStr() ;
            this->writeTransactionRow(tempStringStream.str() );
            tempStringStream.str(std::string());
            tempStringStream.clear();
            tempStringStream << "K,I," << pItem.toStr() ;
            this->writeTransactionRow(tempStringStream.str() );
        }
        itemsLocalCopyMap.insert( std::pair<uint64_t, Item>(pItem.getCode(), pItem) ) ;
        //std::cout << "nf" << std::endl ;
        return true ;
    }
}

long Cart::getItemPrice(Item& pItem, uint64_t pBarcode, unsigned int pBCodeType, bool pPriceChangesWhileShopping)
{
    std::stringstream tempStringStream ;
    std::string barcodeWrkStr ;

	//BOOST_LOG_SEV(my_logger_ca, lt::debug) << "- CA - Brucia all'inferno getItemPrice - " << pItem.getDepartment().getCode();

    if (pBCodeType!=BCODE_EAN13_PRICE_REQ)
    {
        if (!pPriceChangesWhileShopping)
        {
            if ((itemsLocalCopyMap.find(pItem.getCode()) != itemsLocalCopyMap.end()))
            {
                return itemsLocalCopyMap[pItem.getCode()].getPrice();
            } else {
                return pItem.getPrice();
            }
        } else {
            return pItem.getPrice();
        }
    } else {
        tempStringStream.str( std::string() ) ;
        tempStringStream.clear() ;
        tempStringStream << pBarcode ;
        barcodeWrkStr = tempStringStream.str().substr(0,7) + "000000" ;
        return atol(tempStringStream.str().substr(7,5).c_str()) ;
    }
    
}


long Cart::addItemByBarcode( Item& pItem, uint64_t pBarcode, uint32_t pPrice )
{
    long pQtyItem = 1 ;
    return addItemByBarcode(pItem, pBarcode, pQtyItem, pPrice) ;
}

long Cart::addItemByBarcode( Item& pItem, uint64_t pBarcode, uint32_t pQtyItem, uint32_t pPrice )
{
    std::stringstream tempStringStream;
    
    if ( (this->getState()==CART_STATE_TMPFILE_LOADING) || (this->getState()==CART_STATE_READY_FOR_ITEM) )
    {
        try {
            long qtyItem = cartItemsMap[&pItem].quantity + pQtyItem ;
            long qtyBarcode = barcodesMap[pBarcode] + pQtyItem ;
            
            totalsMap[0].itemsNumber = totalsMap[0].itemsNumber + pQtyItem ;
            totalsMap[0].totalAmount = totalsMap[0].totalAmount + ( pPrice * pQtyItem ) ;
            totalsMap[pItem.getDepartmentCode()].itemsNumber = totalsMap[pItem.getDepartmentCode()].itemsNumber + pQtyItem ;
            totalsMap[pItem.getDepartmentCode()].totalAmount = totalsMap[pItem.getDepartmentCode()].totalAmount + ( pPrice * pQtyItem ) ;
            
            itemsNumber = itemsNumber + pQtyItem ;
            
            cartItemsMap[&pItem] = { ITEM, qtyItem } ;
            barcodesMap[pBarcode] = qtyBarcode ;
            
            tempStringStream.str(std::string());
            tempStringStream.clear();
            tempStringStream << this->getState();
            
            BOOST_LOG_SEV(my_logger_ca, lt::info) << "- CART# " << this->number << " - " <<  "Cart state " << tempStringStream.str() ;
            if (this->getState()==CART_STATE_READY_FOR_ITEM)
            {
                tempStringStream.str(std::string());
                tempStringStream.clear();
                tempStringStream << "I,A," << pBarcode << "," << pQtyItem ;
                this->writeTransactionRow(tempStringStream.str() );
            }
        } catch (std::exception const& e)
        {
            BOOST_LOG_SEV(my_logger_ca, lt::error) << "- CART# " << this->number << " - " << "Cart addItemBarcode error: " << e.what();
        }
        return RC_OK ;
    } else {
        return this->state ;
    }
    
}

long Cart::removeItemByBarcode( Item& pItem, uint64_t pBarcode, uint32_t pPrice  )
{
    std::stringstream tempStringStream;
    if ( (this->getState()==CART_STATE_TMPFILE_LOADING) || (this->getState()==CART_STATE_READY_FOR_ITEM) )
    {
        
        long qtyItem ; //= itemsMap[&pItem].quantity ;
        long qtyBarcode ; //= barcodesMap[&pBarcode] ;
        
        if ((cartItemsMap.find(&pItem) == cartItemsMap.end()))
        {
            BOOST_LOG_SEV(my_logger_ca, lt::info) << "- CART# " << this->number << " - " << "Not found: " << pItem.getDescription() ;
            return RC_ERR ;
        }
        else
        {
            BOOST_LOG_SEV(my_logger_ca, lt::info) << "- CART# " << this->number << " - " << "Found: " << pItem.getDescription() ;
            qtyItem = cartItemsMap[&pItem].quantity - 1 ;
            qtyBarcode = barcodesMap[pBarcode] - 1 ;
            totalsMap[0].itemsNumber-- ;
            totalsMap[0].totalAmount = totalsMap[0].totalAmount - pPrice ;
            totalsMap[pItem.getDepartmentCode()].itemsNumber-- ;
            totalsMap[pItem.getDepartmentCode()].totalAmount = totalsMap[pItem.getDepartmentCode()].totalAmount - pPrice;
            
            itemsNumber-- ;
            
            if (qtyItem < 1)
            {
                BOOST_LOG_SEV(my_logger_ca, lt::info) << "- CART# " << this->number << " - " << "Erasing item" ;
                cartItemsMap.erase(&pItem) ;
            } else
            {
                cartItemsMap[&pItem] = { ITEM, qtyItem } ;
            }
            
            if (qtyBarcode < 1)
            {
                barcodesMap.erase(pBarcode) ;
            } else
            {
                barcodesMap[pBarcode] = qtyBarcode ;
            }
            
            if (this->getState()==CART_STATE_READY_FOR_ITEM)
            {
				tempStringStream.str(std::string());
				tempStringStream.clear();
				tempStringStream << "I,V," << pBarcode << "," << "1" ;
				this->writeTransactionRow(tempStringStream.str() );
            }
        }
        return RC_OK ;
    } else {
        return this->state ;
    }
}

long Cart::voidAllCart()
{
    if ( (this->state!=CART_STATE_VOIDED) && (this->state!=CART_STATE_CLOSED) )
    {
        std::stringstream tempStringStream;
        this->state = CART_STATE_VOIDED ;
        tempStringStream.str(std::string());
        tempStringStream.clear();
        tempStringStream << "C,V" ;
        this->writeTransactionRow(tempStringStream.str() );
        return RC_OK ;
    } else {
        return this->state ;
    }
}

long Cart::printCart()
{
    if ( (this->state!=CART_STATE_VOIDED) && (this->state!=CART_STATE_CLOSED) )
    {   typedef std::map<void*, CartRow>::iterator itemRows;
        Item* itmRow ;
        //Totals* totalsRow ;
        
        BOOST_LOG_SEV(my_logger_ca, lt::info) << "- CART# " << this->number << " - " << "Print start" ;
        for(itemRows iterator = cartItemsMap.begin(); iterator != cartItemsMap.end(); iterator++) {
            CartRow key = iterator->second ;
            switch (key.type) {
                case ITEM:
                    itmRow = (Item*)iterator->first;
                    //BOOST_LOG_SEV(my_logger_ca, lt::info) << "- CART# " << this->number << " - " << itmRow->getDescription() << " - " << itmRow->getPrice() << " - " << itmRow->getDepartment().getDescription() << " qty: " << key.quantity ;
                    break;
                case DEPT:
                    break;
                case TOTAL:
                    //totalsRow = (Totals*)iterator->first;
                    //BOOST_LOG_SEV(my_logger_ca, lt::info) << "\nTotale: " << totalsRow->totalAmount << " Item Nr.: " << totalsRow->itemsNumber ;
                    break;
                default:
                    break;
            }
            //BOOST_LOG_SEV(my_logger_ca, lt::info) << "\nkey: " << key ;
        }
        BOOST_LOG_SEV(my_logger_ca, lt::info) << "- CART# " << this->number << " - " << "State: " << this->getState();
        BOOST_LOG_SEV(my_logger_ca, lt::info) << "- CART# " << this->number << " - " << "Totals start" ;
        typedef std::map<uint64_t, Totals>::iterator configurationRows;
        
        for(configurationRows iterator = totalsMap.begin(); iterator != totalsMap.end(); iterator++) {
            BOOST_LOG_SEV(my_logger_ca, lt::info) << "- CART# " << this->number << " - " << "Dept: " << iterator->first << ", value: " << iterator->second.totalAmount << ", items: " << iterator->second.itemsNumber ;
        }
        BOOST_LOG_SEV(my_logger_ca, lt::info) << "- CART# " << this->number << " - " << "Totals end" ;
        BOOST_LOG_SEV(my_logger_ca, lt::info) << "- CART# " << this->number << " - " << "Cart print end" ;
        return RC_OK ;
    } else {
        return this->state ;
    }
}

long Cart::persist( )
{
    typedef std::map<void*, CartRow>::iterator itemRows;
    typedef std::map<uint64_t, long>::iterator barcodesRows;
    uint64_t rowBarcode ;
    long qty = 0 ;
    
    BOOST_LOG_SEV(my_logger_ca, lt::info) << "- CART# " << this->number << " - " << "cartFileName: " << cartFileName ;
    
    std::ofstream cartFile( cartFileName.c_str() );
    
    for(barcodesRows iterator = barcodesMap.begin(); iterator != barcodesMap.end(); iterator++)
    {
        rowBarcode = (uint64_t)iterator->first;
        qty = iterator->second ;
        cartFile << rowBarcode << "," << qty << "\n";
    }
    
    cartFile.close() ;
    return RC_OK ;
}

long Cart::sendToPos( uint32_t pPosNumber, string pScanInPath, string pStoreId )
{
    if (this->getState()==CART_STATE_READY_FOR_ITEM)
    {
        cart_sendtopos_mutex.lock() ;
        
        typedef std::map<void*, CartRow>::iterator itemRows;
        typedef std::map<uint64_t, long>::iterator barcodesRows;
        std::stringstream tempStringStream;
        
        tempStringStream.str(std::string());
        tempStringStream.clear();
        
        uint64_t rowBarcode ;
        long qty = 0 ;
        string scanInTmpFileName = (boost::format("%s/POS%03lu.TMP") % pScanInPath % pPosNumber).str();
        string scanInFileName = (boost::format("%s/POS%03lu.IN") % pScanInPath % pPosNumber).str();
        BOOST_LOG_SEV(my_logger_ca, lt::info) << "- CART# " << this->number << " - " << "Sending to pos with file: " << cartFileName ;
        boost::posix_time::time_facet *facet = new boost::posix_time::time_facet("%H%M%S%Y%m%d");
        std::stringstream date_stream;
        date_stream.imbue(std::locale(date_stream.getloc(), facet));
        date_stream << boost::posix_time::microsec_clock::universal_time();
        std::ofstream scanInFile( scanInTmpFileName.c_str() );
        
        tempStringStream << loyCardsMap.begin()->second ;
        
        scanInFile << "03;" << date_stream.str() //tsInit
        << ";" << date_stream.str() //tsEnd
        << ";" << pPosNumber
        << ";" << tempStringStream.str().substr(1,8) //customerCode
        << ";" << pStoreId //serverId
        << ";" << this->number
        << ";" << "0"
        << ";" << this->itemsNumber
        << ";" << totalsMap[0].totalAmount
        << ";" << totalsMap[0].totalAmount
        << ";" << "0" << loyCardsMap.begin()->second
        << endl;
        
        for(barcodesRows iterator = barcodesMap.begin(); iterator != barcodesMap.end(); iterator++)
        {
            rowBarcode = (uint64_t)iterator->first;
            qty = iterator->second ;
            if (qty > 0)
            {
                for (long repetitions = 0; repetitions < qty; repetitions++ )
                {
                    scanInFile << "04;" << rowBarcode << ";" << "0.00" << ";" << date_stream.str() << endl;
                }
            }
        }
        
        scanInFile.close() ;
        
        boost::filesystem::rename(scanInTmpFileName.c_str(), scanInFileName);
        
        cart_sendtopos_mutex.unlock() ;
        
        return RC_OK ;
    } else {
        return this->state ;
    }
}

string Cart::getAllCartJson( ArchiveMap<Item>& pAllItemsMap, bool pWithBarcodes )
{
    typedef std::map<unsigned int, uint64_t>::iterator loyCardRows ;
    
    std::stringstream tempStringStream;
    
    tempStringStream.str(std::string());
    tempStringStream.clear();
    
    uint64_t rowIntCode, rowCardCode ;
    long qty = 0 ;
    bool firstRow = true ;
    
	BOOST_LOG_SEV(my_logger_ca, lt::info) << "- CART# " << this->number << " - Getting full cart in JSON format" ;
    
    
    tempStringStream << "{\"loyCards\":" ;
    firstRow = true ;
    for(loyCardRows iterator = loyCardsMap.begin(); iterator != loyCardsMap.end(); iterator++)
    {
        if (!firstRow)
        {
            tempStringStream << "," ;
        }
        rowCardCode = (uint64_t)iterator->second;
        tempStringStream << "{\"loyCard\":" << rowCardCode << "}" ;
        firstRow = false ;
    }
    //itemsMap[&pItem] = { ITEM, qtyItem } ;
    firstRow = true ;
    
    if (pWithBarcodes)
    {
        typedef std::map<uint64_t, long>::iterator barcodesRows ;
        uint64_t rowBarCode ;
        tempStringStream << ",\"barcodes\":{" ;
        for(barcodesRows iterator = barcodesMap.begin(); iterator != barcodesMap.end(); iterator++)
        {
            rowBarCode = iterator->first ;
            qty = iterator->second ;
            if (qty > 0)
            {
                if (!firstRow)
                {
                    tempStringStream << "," ;
                }
                tempStringStream << "\"barcode\":{\"code\":" << rowBarCode << ",\"qty\":" << qty << "}" << endl;
                firstRow = false ;
            }
        }
    } else {
        typedef std::map<void*, CartRow>::iterator itemRows ;
        Item* tempItm ;
        tempStringStream << ",\"items\":{" ;

        for(itemRows iterator = cartItemsMap.begin(); iterator != cartItemsMap.end(); iterator++)
        {
            tempItm = (Item*)iterator->first ;
            rowIntCode= tempItm->getCode() ;
            qty = iterator->second.quantity ;
            if (qty > 0)
            {
                if (!firstRow)
                {
                    tempStringStream << "," ;
                }
                tempStringStream << "\"item\":{\"intCode\":" << rowIntCode << ",\"description\":\"" << pAllItemsMap[rowIntCode].getDescription() << "\",\"qty\":" << qty << "}" << endl;
                firstRow = false ;
            }
        }
    }
    tempStringStream << "}}" ;
    
    return tempStringStream.str() ;
}

std::map <uint64_t, Totals> Cart::getTotals()
{
    return totalsMap ;
}

long Cart::close()
{
    //tmpTransactionFile.close() ;
    state = CART_STATE_CLOSED ;
    return RC_OK ;
}