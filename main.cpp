#include <iostream>
#include <iostream>
#include <cstdint>
#include "Logger.h"
#include "BigInt.h"
#include "Field.h"
#include "IntegerMod.h"
#include "Rand.h"
#include <tuple>
#include <stdio.h>
constexpr char const* file = "config.json";
struct config
{
std::string prime;
std::string K;
std::string N;
std::string LogLevel;
};
std::string find(std::string const& str, std::string const& key)
{
    auto k = str.find(key);
    if(k == std::string::npos)
    {
        throw error::ExeptionBase<error::ErrorList::InputError>("Lose mandatory field " + key);
    }
    k = str.find(':', k + 1);
    if(k == std::string::npos)
    {
        throw error::ExeptionBase<error::ErrorList::InputError>("Lose mandatory value of field " + key);
    }
    auto start = str.find("\"",k + 1);
    auto end   = str.find( "\"", start + 1);
    if(end == str.size())
        end  = str.find( "\n", start + 1);

    return str.substr(start + 1, end - start - 1);
};
config parse(std::string const& str)
{
    config cfg;
    cfg.prime = find(str,"prime");
    cfg.K = find(str,"K");
    cfg.N = find(str,"N");
    cfg.LogLevel = find(str,"LogLevel");
    return cfg;
}


int main()
{
    Rand::reset();
    Logger::setLevel(Log::Level::debug);
    bool crash = false;

    try {
        Logger::start();
    }
    catch (error::Exeption &exp) {
        std::cout << exp.what() << std::endl;
        return -1;
    }
    std::fstream in(file, std::ios_base::in);

    if (!in.is_open()) {
        Logger::print<Log::Level::fatal>((std::string("Can't open file: ") + file).c_str());
        crash = true;
    }

    std::stringstream buf;
    buf << in.rdbuf();
    std::string json = buf.str();
    BigInteger::BigInt prime;
    using namespace BigInteger;
    BigInteger::BigInt k = "0"_BigInt, n = "0"_BigInt;
    in.close();
    try {
        auto cfg = parse(json);
        Logger::setLevel(cfg.LogLevel);
        prime.setByString(cfg.prime);
        k.setByString(cfg.K);
        n.setByString(cfg.N);
    }
    catch (error::Exeption &exp) {
        Logger::print<Log::Level::fatal>(exp.what().c_str());
        std::cout << exp.what() << std::endl;
        crash = true;
    }
    if(k == "0"_BigInt && n < k)
    {
        crash = true;
        Logger::print<Log::Level::fatal>("k == 0 or n < k");
    }
    if (crash) {
        Logger::setLevel(Log::Level::debug);
        Logger::stop();
        return -1;
    }

    Field::IntegerMod::setIntegerMod(prime);
    std::vector<Field::BigInt> Ai;Field::BigInt ai;
    std::vector<Field::BigInt> User;
    std::cout << "Please, write shared secret in hex format" << std::endl;
    std::string str;
    std::cin >> str;
    ai.setByString(str);
    Ai.push_back(ai);
    Logger::print<Log::Level::info>("Ai[%d]=%s", 0, Ai[0].toString().c_str());

    using namespace BigInteger;
    for(BigInteger::BigInt all = "1"_BigInt; all < k; all++)
    {
        Ai.push_back(Field::BigInt(Rand::rand(prime.getBitSize())));
        Logger::print<Log::Level::info>("Ai[%s]=%s", all.toString().c_str(), Ai.back().toString().c_str());
    }
    std::vector<Field::BigInt> Keys;
    using namespace Field;
    std::vector<int> user;
    BigInteger::BigInt  first = "0"_BigInt;
    int I = 0;
    for(BigInteger::BigInt i = first; i < n + first; i++, I++)
    {
        Field::BigInt res = "0"_BigIntMod;
        int J = 0;
        for(BigInteger::BigInt j = "0"_BigIntMod; j < k; j++, J++)
        {
            res += (Field::BigInt(i) ^ Field::BigInt(j % (prime - "1"_BigInt))) * Ai[J];
        }
        Logger::print<Log::Level::info>("Keys[%s]=%s", i.toString().c_str(), res.toString().c_str());
        Keys.push_back(res);
        User.push_back(Field::BigInt(i));
        user.push_back(I);
    }

    std::cout << "tuple of key pieces already generated." << std::endl;
    while(true)
    {
        int mode = 0;
        std::cout << "1. Try to restore secret" << std::endl;
        std::cout << "0. Exit" << std::endl;
        std::cin >> mode;
        if(mode == 0)
            break;
        else
        {
            std::cout << "Write all numbers used for restoring" << std::endl << std::flush;
            std::string nk;
            //std::cin.clear();
            std::getline(std::cin, nk);
            std::getline(std::cin, nk);
            std::vector<uint64_t> index;
            while(nk.size() > 0)
            {
                auto n = nk.find(' ');
                if(n == std::string::npos)
                    n = nk.size();
                auto sub = nk.substr(0, n);
                int N = 0;
                auto t = sscanf(sub.c_str(), "%d", &N);
                if(t < 0)
                {
                    nk.clear();
                    index.resize(0);
                    std::cout << "Error in input values" << std::endl;
                    Logger::print<Log::Level::error>("Error in input values");
                }else
                {
                    index.push_back(N);
                    if(n != nk.size())
                        nk.erase(0, n + 1);
                    else
                        nk.clear();
                }
            }
            if(index.size() < Ai.size())
            {
                std::cout << "Error to few pieces of keys" << std::endl;
                Logger::print<Log::Level::error>("Error to few pieces of keys");
            }else
            {
                Field::BigInt M = "0"_BigIntMod;
                for(size_t i = 0; i < Ai.size(); i++)
                {
                    Field::BigInt P = "1"_BigIntMod;
                    for(size_t j = 0; j < Ai.size(); j++)
                    {
                        if(User[index[i]] != User[index[j]]) {
                            P = P * ("0"_BigIntMod - User[index[j]]) / (User[index[i]] - User[index[j]]);
                        }
                    }
                    M = M + P * Keys[user[index[i]]];
                    Logger::print<Log::Level::debug>("M[%lu]=%s", i, M.toString().c_str());
                }
                std::cout << "Secret: " << M.toString() << std::endl;
            }
        }
    }

    Logger::print<Log::Level::debug>((std::string("Json config input: \n") + json).c_str());
    Logger::setLevel(Log::Level::debug);
    Logger::stop();
    return 0;
}