#pragma once
#include <list>
#include <string>
#include <unordered_set>

namespace Ninja
{
    struct Product
    {
        std::string name = {};
        std::string id = {};
        std::string installed_version = {};
        std::string available_version = {};
        std::string source = {};
    };
    using ProductId = std::string;
    using ProductCollection = std::list<Product>;

    using Version = std::string;
    using ProductVersionCollection = std::list<Version>;

    using FilterProduct = std::unordered_set<ProductId>;
    using FilterVersion = std::unordered_set<Version>;
}