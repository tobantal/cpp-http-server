#include "adapters/primary/SchemaValidator.hpp"
#include "ports/input/IJsonProcessor.hpp"
#include "ports/output/IEnvironment.hpp"
#include "domain/error/BadRequestError.hpp"
#include "domain/error/ConvertError.hpp"
#include <sstream>
#include <string>

SchemaValidator::SchemaValidator(std::shared_ptr<Schema> schema)
    : schema_(std::move(schema))
{
}

std::string SchemaValidator::name() const
{
    return "SchemaValidator";
}

void SchemaValidator::handle(IRequest& req, IResponse& /*res*/)
{
    auto obj = req.getObject(IJsonProcessor::JSON_OBJECT_KEY);
    if (!obj.has_value())
    {
        throw BadRequestError("Request body must be JSON");
    }

    auto env = std::static_pointer_cast<IEnvironment>(*obj);

    std::vector<std::string> errors;

    for (const auto& field : schema_->fields())
    {
        if (!env->hasProperty(field.name))
        {
            if (field.required)
            {
                errors.push_back(field.name + " is required");
            }
            continue;
        }

        bool typeOk = true;
        if (field.type == typeid(std::string))
        {
            try { env->get<std::string>(field.name); }
            catch (const ConvertError&) { errors.push_back(field.name + " must be a string"); typeOk = false; }
        }
        else if (field.type == typeid(int))
        {
            try { env->get<int>(field.name); }
            catch (const ConvertError&) { errors.push_back(field.name + " must be an integer"); typeOk = false; }
        }
        else if (field.type == typeid(double))
        {
            try { env->get<double>(field.name); }
            catch (const ConvertError&) { errors.push_back(field.name + " must be a number"); typeOk = false; }
        }
        else if (field.type == typeid(bool))
        {
            try { env->get<bool>(field.name); }
            catch (const ConvertError&) { errors.push_back(field.name + " must be a boolean"); typeOk = false; }
        }

        if (!typeOk)
        {
            continue;
        }

        if (field.hasMinLength)
        {
            try
            {
                auto val = env->get<std::string>(field.name);
                if (val.length() < field.minLength)
                {
                    errors.push_back(field.name + " minLength is " + std::to_string(field.minLength));
                }
            }
            catch (const ConvertError&) {}
        }

        if (field.hasMaxLength)
        {
            try
            {
                auto val = env->get<std::string>(field.name);
                if (val.length() > field.maxLength)
                {
                    errors.push_back(field.name + " maxLength is " + std::to_string(field.maxLength));
                }
            }
            catch (const ConvertError&) {}
        }

        if (field.hasMin)
        {
            if (field.type == typeid(int))
            {
                try { if (env->get<int>(field.name) < static_cast<int>(field.minVal)) errors.push_back(field.name + " must be >= " + std::to_string(static_cast<int>(field.minVal))); }
                catch (const ConvertError&) {}
            }
            else if (field.type == typeid(double))
            {
                try { if (env->get<double>(field.name) < field.minVal) errors.push_back(field.name + " must be >= " + std::to_string(field.minVal)); }
                catch (const ConvertError&) {}
            }
        }

        if (field.hasMax)
        {
            if (field.type == typeid(int))
            {
                try { if (env->get<int>(field.name) > static_cast<int>(field.maxVal)) errors.push_back(field.name + " must be <= " + std::to_string(static_cast<int>(field.maxVal))); }
                catch (const ConvertError&) {}
            }
            else if (field.type == typeid(double))
            {
                try { if (env->get<double>(field.name) > field.maxVal) errors.push_back(field.name + " must be <= " + std::to_string(field.maxVal)); }
                catch (const ConvertError&) {}
            }
        }
    }

    if (!errors.empty())
    {
        std::string msg = "Validation failed: ";
        for (size_t i = 0; i < errors.size(); ++i)
        {
            if (i > 0) msg += "; ";
            msg += errors[i];
        }
        throw BadRequestError(msg);
    }
}