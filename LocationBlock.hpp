#pragma once

#include <string> //| std::string
#include <vector> //| std::vector
#include "ParserConfigFile.hpp" //| ParserConfigFile

#include <iostream> //| Para testes, remover depois


class LocationBlock
{
    public:
        LocationBlock();
        ~LocationBlock();

        void addLocationBlock(std::vector<std::string> &tokens);

        //| Getters
        bool getAutoIndex() const;
        bool getCanUpload() const;
        std::string getUri() const;
        std::string getAlias() const;
        std::string getReturn() const;
        std::string getUploadPath() const;
        std::vector<std::string> getIndex() const;
        std::vector<std::string> getCgiExtensions() const;
        std::vector<std::string> getAllowMethods() const;

    private:
        bool _autoIndex;
        bool _canUpload;
        std::string _uri;
        std::string _alias;
        std::string _return;
        std::string _uploadPath;
        std::vector<std::string> _index;
        std::vector<std::string> _cgiExtensions;
        std::vector<std::string> _allowMethods;

        void printLocationBlock();

        void addAutoIndex(std::vector<std::string> &tokens);
        void addCanUpload(std::vector<std::string> &tokens);
        void addUri(std::vector<std::string> &tokens);
        void addAlias(std::vector<std::string> &tokens);
        void addReturn(std::vector<std::string> &tokens);
        void addUploadPath(std::vector<std::string> &tokens);
        void addIndex(std::vector<std::string> &tokens);
        void addCgiExtensions(std::vector<std::string> &tokens);
        void addAllowMethods(std::vector<std::string> &tokens);
};