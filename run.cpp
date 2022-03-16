#include "Flight.h"
#include "Seller.h"

int
main()
{
    Flight flight1("001","CA","NY");
    Flight flight2("002","NY","AK");
    Seller seller1("001");
    Seller seller2("002");

    seller1.package_pkg("1","001","CA","AK");
    seller1.package_pkg("2","001","NY","AK");
    seller1.package_pkg("3","002","CA","XX");
    seller1.check_data();
    seller2.package_pkg("100","001","CA","NY");
    seller2.package_pkg("101","002","CA","XX");
    seller2.check_data();

    flight1.load_package("100");
    flight1.load_package("1");
    flight1.load_package("3");
    // flight1.load_package("2");
    // flight1.load_package("1");
    flight1.check_data();
    flight1.landed();
    flight1.check_data();

    
    flight2.load_package("1");
    flight2.load_package("2");
    flight2.check_data();
    flight2.landed();
    flight2.check_data();

    seller1.check_data();
    seller2.check_data();

    return 0;
}