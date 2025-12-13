#pragma once

# include "WebservHeader.hpp"

class ConfigFile;

class LocationBlock {
    private:
        ConfigFile                  &_config;
        bool                        _autoIndex;
        bool                        _canUpload;
        std::string                 _uri;
        std::string                 _alias;
        std::string                 _return;
        std::string                 _uploadPath;
        std::vector<std::string>    _index;
        std::vector<std::string>    _cgiExtensions;
        std::vector<std::string>    _allowMethods;
        bool                        _cookiesEnabled;

        
        void addAutoIndex();
        void addCanUpload();
        void addUri();
        void addAlias();
        void addReturn();
        void addUploadPath();
        void addIndex();
        void addCgiExtensions();
        void addAllowMethods();
        void addCookiesEnabled();
        
    public:
        LocationBlock(ConfigFile &config);
        ~LocationBlock();

        LocationBlock &operator=(const LocationBlock &src);
        
        void printLocationBlock();
        void addLocationBlock();
        bool validatePath(const std::string &path) const;

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
        bool getCookiesEnabled() const;
        std::string getPath(std::string root) const;
};