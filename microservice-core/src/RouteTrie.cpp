#include "application/RouteTrie.hpp"
#include "adapters/primary/RouteMatcher.hpp"
#include <algorithm>

RouteTrie::RouteTrie()
    : root_(std::make_shared<TrieNode>())
{
}

void RouteTrie::insert(const std::string &pattern,
                        const std::string &method,
                        std::shared_ptr<IHttpHandler> handler)
{
    auto segments = splitPath(pattern);
    auto current = root_.get();

    for (const auto &segment : segments)
    {
        if (segment == "*")
        {
            if (!current->wildcardChild)
            {
                current->wildcardChild = std::make_unique<TrieNode>();
            }
            current = current->wildcardChild.get();
        }
        else if (RouteMatcher::isNamedParam(segment))
        {
            if (!current->paramChild)
            {
                current->paramChild = std::make_unique<TrieNode>();
                current->paramChild->paramName = RouteMatcher::paramName(segment);
            }
            current = current->paramChild.get();
        }
        else
        {
            auto it = current->children.find(segment);
            if (it == current->children.end())
            {
                auto child = std::make_shared<TrieNode>();
                current->children[segment] = child;
                current = child.get();
            }
            else
            {
                current = it->second.get();
            }
        }
    }

    current->handlers[method] = std::move(handler);
}

std::optional<RouteMatch> RouteTrie::lookup(const std::string &method,
                                             const std::string &path) const
{
    auto result = traverse(path);
    if (!result)
    {
        return std::nullopt;
    }

    auto methodIt = result->node->handlers.find(method);
    if (methodIt == result->node->handlers.end())
    {
        return std::nullopt;
    }

    RouteMatch match;
    match.handler = methodIt->second;
    match.pattern = result->pattern;
    match.pathParams = result->pathParams;
    return match;
}

bool RouteTrie::lookupAny(const std::string &path) const
{
    auto result = traverse(path);
    return result.has_value() && !result->node->handlers.empty();
}

std::vector<std::string> RouteTrie::lookupMethods(const std::string &path) const
{
    auto result = traverse(path);
    if (!result)
    {
        return {};
    }

    std::vector<std::string> methods;
    for (const auto &[method, _] : result->node->handlers)
    {
        methods.push_back(method);
    }
    return methods;
}

std::optional<RouteTrie::LookupResult> RouteTrie::traverse(const std::string &path) const
{
    auto segments = splitPath(path);

    struct State
    {
        TrieNode *node;
        size_t segmentIndex;
        std::map<std::string, std::string> pathParams;
        std::string pattern;
    };

    std::vector<State> stack;
    stack.push_back({root_.get(), 0, {}, ""});

    while (!stack.empty())
    {
        State current = std::move(stack.back());
        stack.pop_back();

        if (current.segmentIndex == segments.size())
        {
            if (!current.node->handlers.empty())
            {
                LookupResult result;
                result.node = std::shared_ptr<TrieNode>(root_, current.node);
                result.pathParams = std::move(current.pathParams);
                result.pattern = std::move(current.pattern);
                return result;
            }
            continue;
        }

        const std::string &segment = segments[current.segmentIndex];
        std::string patternSoFar = current.pattern;

        if (current.node->wildcardChild)
        {
            State next;
            next.node = current.node->wildcardChild.get();
            next.segmentIndex = current.segmentIndex + 1;
            next.pathParams = current.pathParams;
            next.pattern = patternSoFar + "/*";
            stack.push_back(std::move(next));
        }

        if (current.node->paramChild)
        {
            State next;
            next.node = current.node->paramChild.get();
            next.segmentIndex = current.segmentIndex + 1;
            next.pathParams = current.pathParams;
            next.pathParams[current.node->paramChild->paramName] = segment;
            next.pattern = patternSoFar + "/:" + current.node->paramChild->paramName;
            stack.push_back(std::move(next));
        }

        if (auto it = current.node->children.find(segment); it != current.node->children.end())
        {
            State next;
            next.node = it->second.get();
            next.segmentIndex = current.segmentIndex + 1;
            next.pathParams = current.pathParams;
            next.pattern = patternSoFar + "/" + segment;
            stack.push_back(std::move(next));
        }
    }

    return std::nullopt;
}

std::vector<std::string> RouteTrie::splitPath(const std::string &path)
{
    std::vector<std::string> segments;
    std::string segment;

    for (char ch : path)
    {
        if (ch == '/')
        {
            if (!segment.empty())
            {
                segments.push_back(segment);
                segment.clear();
            }
        }
        else
        {
            segment += ch;
        }
    }

    if (!segment.empty())
    {
        segments.push_back(segment);
    }

    return segments;
}
