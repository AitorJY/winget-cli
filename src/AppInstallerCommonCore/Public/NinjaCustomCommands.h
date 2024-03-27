#pragma once
#include "../../WindowsPackageManager/WindowsPackageManager.h"
#include "NinjaProductCollection.h"
#include "AppInstallerErrors.h"
#include "../../Valijson/valijson/thirdparty/nlohmann-json-3.1.2/nlohmann/json.hpp"
#include <list>

namespace Ninja
{

    extern ProductCollection products_cached;
    extern ProductVersionCollection product_versions_cached;
    extern FilterProduct filter_products_cached;
    extern FilterVersion filter_versions_cached;

    enum class Command
    {
        Install = 0,
        List = 1,
        Show = 2,
        Settings = 3
    };

    constexpr int kErrorParseRequest = 5001;

    int WingetInvoke(const std::string &request, char **response);
    int WingetFree(char *output);

    int List(const nlohmann::json &request, char **response);
    int Install(const nlohmann::json &request, char **response);
    int Show(const nlohmann::json &request, char **response);
    int Settings(const nlohmann::json &request, char **response);
    int ExecuteCommand(const std::wstring &command, const std::vector<std::wstring> &args);

    void LoadFilterVersion(const nlohmann::json &request);
    void LoadFilterProduct(const nlohmann::json &request);
    template <typename T>
    void LoadFilter(const nlohmann::json &request, const std::string &name, T &filter);

    std::vector<std::wstring> ToWstring(const std::vector<std::string> &values);
    nlohmann::json CreateJsonError(int code, const std::string &message);
    void SetResponse(const std::string &message, char **response);
    void SetErrorResponse(int error_code, const std::string &error_message, char **response);

    int WingetInvoke(const std::string &request, char **response)
    {
        const auto kRequest = nlohmann::json::parse(request);
        if (kRequest.empty())
        {
            SetErrorResponse(kErrorParseRequest, "Failed to parse the json request", response);
            return kErrorParseRequest;
        }

        if (kRequest.at("command") == Command::Install)
        {
            return Ninja::Install(kRequest, response);
        }
        else if (kRequest.at("command") == Command::List)
        {
            return Ninja::List(kRequest, response);
        }
        else if (kRequest.at("command") == Command::Show)
        {
            return Ninja::Show(kRequest, response);
        }
        else if (kRequest.at("command") == Command::Settings)
        {
            return Ninja::Settings(kRequest, response);
        }
        else
        {
            return E_INVALIDARG;
        }
    }

    int WingetFree(char *output)
    {
        if (output != nullptr)
        {
            free(output);
        }
        products_cached.clear();
        product_versions_cached.clear();
        filter_products_cached.clear();
        filter_versions_cached.clear();
        return S_OK;
    }

    int Install(const nlohmann::json &request, char **response)
    {
        const auto kCmdResult = ExecuteCommand(L"install", ToWstring(request.at("arguments")));
        if (kCmdResult != S_OK)
        {
            SetErrorResponse(kCmdResult, AppInstaller::GetUserPresentableMessage(kCmdResult), response);
            return kCmdResult;
        }
        SetErrorResponse(S_OK, "", response);
        return S_OK;
    }

    int List(const nlohmann::json &request, char **response)
    {
        products_cached.clear();
        LoadFilterProduct(request);

        const auto kCmdResult = ExecuteCommand(L"list", ToWstring(request.at("arguments")));
        if (kCmdResult != S_OK)
        {
            SetErrorResponse(kCmdResult, AppInstaller::GetUserPresentableMessage(kCmdResult), response);
            return kCmdResult;
        }

        nlohmann::json json;
        json["products"] = nlohmann::json::array();
        for (const auto &kProduct : products_cached)
        {
            nlohmann::json product;
            product["name"] = kProduct.name;
            product["id"] = kProduct.id;
            product["source"] = kProduct.source;
            product["installedVersion"] = kProduct.installed_version;
            product["availableVersion"] = kProduct.available_version;
            json["products"].push_back(product);
        }

        json["error"] = CreateJsonError(S_OK, "");
        SetResponse(json.dump(), response);
        return S_OK;
    }

    int Show(const nlohmann::json &request, char **response)
    {
        product_versions_cached.clear();
        LoadFilterVersion(request);

        const auto kCmdResult = ExecuteCommand(L"show", ToWstring(request.at("arguments")));
        if (kCmdResult != S_OK)
        {
            SetErrorResponse(kCmdResult, AppInstaller::GetUserPresentableMessage(kCmdResult), response);
            return kCmdResult;
        }

        nlohmann::json json;
        json["versions"] = product_versions_cached;
        json["error"] = CreateJsonError(S_OK, "");
        SetResponse(json.dump(), response);
        return S_OK;
    }

    int Settings(const nlohmann::json &request, char **response)
    {
        const auto kCmdResult = ExecuteCommand(L"settings", ToWstring(request.at("arguments")));
        if (kCmdResult != S_OK)
        {
            SetErrorResponse(kCmdResult, AppInstaller::GetUserPresentableMessage(kCmdResult), response);
            return kCmdResult;
        }
        SetErrorResponse(S_OK, "", response);
        return S_OK;
    }

    int ExecuteCommand(const std::wstring &command, const std::vector<std::wstring> &args)
    {
        auto cmd = std::vector<const wchar_t *>{L"winget", command.c_str()};
        std::transform(args.begin(), args.end(), back_inserter(cmd), [](const std::wstring &str)
                       { return str.c_str(); });
        return WindowsPackageManagerCLIMain(cmd.size(), cmd.data());
    }

    void LoadFilterVersion(const nlohmann::json &request)
    {
        LoadFilter(request, "versions", filter_versions_cached);
    }

    void LoadFilterProduct(const nlohmann::json &request)
    {
        LoadFilter(request, "products", filter_products_cached);
    }

    template <typename T>
    void LoadFilter(const nlohmann::json &request, const std::string &name, T &filter)
    {
        filter.clear();
        if (request.find("filters") == request.end())
        {
            return;
        }
        if (request.at("filters").find(name) == request.at("filters").end())
        {
            return;
        }
        for (const auto &kVersion : request.at("filters").at(name))
        {
            filter.emplace(kVersion.get<std::string>());
        }
    }

    std::vector<std::wstring> ToWstring(const std::vector<std::string> &values)
    {
        auto wvalues = std::vector<std::wstring>{};
        std::transform(values.begin(), values.end(), back_inserter(wvalues), [](const std::string &str)
                       { return std::wstring{str.begin(), str.end()}; });
        return wvalues;
    }

    void SetErrorResponse(int error_code, const std::string &error_message, char **response)
    {
        nlohmann::json json;
        json["error"] = CreateJsonError(error_code, error_message);
        SetResponse(json.dump(), response);
    }

    nlohmann::json CreateJsonError(int code, const std::string &message)
    {
        nlohmann::json json;
        json["code"] = code;
        json["message"] = message;
        return json;
    }

    void SetResponse(const std::string &message, char **response)
    {
        char *pvalue = _strdup(message.c_str());
        *response = pvalue;
    }
}