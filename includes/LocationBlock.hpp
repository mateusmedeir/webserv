#pragma once

# include "WebservHeader.hpp"

class ConfigFile;

class LocationBlock {
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

        
        void addAutoIndex(ConfigFile &config);
        void addCanUpload(ConfigFile &config);
        void addUri(ConfigFile &config);
        void addAlias(ConfigFile &config);
        void addReturn(ConfigFile &config);
        void addUploadPath(ConfigFile &config);
        void addIndex(ConfigFile &config);
        void addCgiExtensions(ConfigFile &config);
        void addAllowMethods(ConfigFile &config);
        
    public:
        LocationBlock();
        ~LocationBlock();
        
        void printLocationBlock();
        void addLocationBlock(ConfigFile &config);

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
};