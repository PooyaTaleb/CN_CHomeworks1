//              append to log file {
#include <fstream>

int main() {  
  std::ofstream outfile;

  outfile.open("test.txt", std::ios_base::app); // append instead of overwrite
  outfile << "Data"; 
  return 0;
}
//              }

// for json
    // https://github.com/nlohmann/json

    // read
        #include <fstream>
        #include <nlohmann/json.hpp>
        using json = nlohmann::json;
        // ...
        std::ifstream f("example.json");
        json data = json::parse(f);


// ftp lib
// boost::asio. Boost is a very well-know set of libraries for C++, and asio is the part implementing the support for networking. 
// https://www.boost.org/doc/libs/1_49_0/doc/html/boost_asio.html

// netstat -ab to see available ports (pls use 42069)