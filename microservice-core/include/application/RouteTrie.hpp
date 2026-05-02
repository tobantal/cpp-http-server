#pragma once

#include "ports/input/IHttpHandler.hpp"
#include <map>
#include <memory>
#include <optional>
#include <string>
#include <vector>

/**
 * @file RouteTrie.hpp
 * @brief Trie-based router for O(k) path lookup
 * @author Anton Tobolkin
 */

struct TrieNode
{
    std::map<std::string, std::shared_ptr<TrieNode>> children;
    std::unique_ptr<TrieNode> paramChild;
    std::unique_ptr<TrieNode> wildcardChild;
    std::map<std::string, std::shared_ptr<IHttpHandler>> handlers;
    std::string paramName;
};

struct RouteMatch
{
    std::shared_ptr<IHttpHandler> handler;
    std::string pattern;
    std::map<std::string, std::string> pathParams;
};

/**
 * @class RouteTrie
 * @brief Trie-based router supporting static, :param, and * segments
 */
class RouteTrie
{
public:
    RouteTrie();

    /**
     * @brief Insert a route pattern with method and handler
     * @param pattern URL pattern (e.g., /api/users/:id, /files/*)
     * @param method HTTP method (GET, POST, etc.)
     * @param handler Handler to invoke on match
     */
    void insert(const std::string &pattern,
                const std::string &method,
                std::shared_ptr<IHttpHandler> handler);

    /**
     * @brief Lookup handler by method and path
     * @param method HTTP method
     * @param path Request path
     * @return RouteMatch with handler, pattern, and pathParams, or nullopt
     */
    std::optional<RouteMatch> lookup(const std::string &method,
                                     const std::string &path) const;

    /**
     * @brief Check if any handler exists for the given path
     * @param path Request path
     * @return true if any handler matches
     */
    bool lookupAny(const std::string &path) const;

    /**
     * @brief Get all HTTP methods allowed for the given path
     * @param path Request path
     * @return Vector of method strings
     */
    std::vector<std::string> lookupMethods(const std::string &path) const;

private:
    std::shared_ptr<TrieNode> root_;

    static std::vector<std::string> splitPath(const std::string &path);

    struct LookupResult
    {
        std::shared_ptr<TrieNode> node;
        std::map<std::string, std::string> pathParams;
        std::string pattern;
    };

    std::optional<LookupResult> traverse(const std::string &path) const;

    static void buildPattern(const std::shared_ptr<TrieNode> &node,
                             const std::vector<std::string> &segments,
                             std::string &pattern);
};
