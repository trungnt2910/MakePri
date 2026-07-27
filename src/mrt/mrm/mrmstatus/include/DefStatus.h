#pragma once

#include <cstdint>

#include <mrm/common/Base.h>

#include <windows.h>

#include <list>
#include <memory>
#include <string>
#include <vector>

namespace Microsoft::Resources
{

class IDefStatus
{
public:
    virtual ~IDefStatus() = default;
    [[nodiscard]] virtual HRESULT GetWhat() const = 0;
    [[nodiscard]] virtual const wchar_t* GetWhere() const = 0;
    [[nodiscard]] virtual const wchar_t* GetDesc() const = 0;
    [[nodiscard]] virtual std::uint32_t GetLine() const = 0;
    [[nodiscard]] virtual const DEFSTATUS* GetDefStatus() const = 0;
    [[nodiscard]] virtual bool Failed() const = 0;
    [[nodiscard]] virtual bool Succeeded() const = 0;
    virtual bool SetError(
        HRESULT error,
        const wchar_t* where,
        std::uint32_t line,
        const wchar_t* description,
        std::uint32_t descriptionLength) = 0;
};

class IDefStatusEx : public IDefStatus
{
public:
    ~IDefStatusEx() override = default;

    [[nodiscard]] virtual bool Succeeded() const override = 0;
    virtual void Reset() = 0;
    [[nodiscard]] virtual HRESULT GetHResult() = 0;
    [[nodiscard]] virtual bool Failed() const override = 0;
    [[nodiscard]] virtual const std::vector<const IDefStatus*>& GetWarningList() const = 0;
    virtual void SetErrorLocation(const wchar_t* location) = 0;
    virtual void ResetErrorLocation() = 0;
    virtual bool SetError(
        HRESULT error,
        const wchar_t* file,
        std::uint32_t line,
        const wchar_t* description,
        std::uint32_t descriptionLength) override = 0;
    virtual bool SetError(HRESULT error, const wchar_t* where, std::uint32_t line, const wchar_t* description) = 0;
    virtual bool SetError(HRESULT error, const wchar_t* description) = 0;
    virtual bool AddWarning(HRESULT warning, const wchar_t* description) = 0;
    virtual bool AddWarning(HRESULT warning, const wchar_t* where, std::uint32_t line, const wchar_t* description) = 0;
    virtual bool AddWarning(
        HRESULT warning,
        const wchar_t* where,
        std::uint32_t line,
        const wchar_t* description,
        std::uint32_t descriptionLength) = 0;
    virtual bool DiagnosticLogA(const char* format, ...) = 0;
    virtual bool DiagnosticLogWithPrefixA(const char* prefix, const char* format, ...) = 0;
    virtual bool DiagnosticLogWithErrorCodeA(const char* message, HRESULT error) = 0;
};

bool Def_HrFailed0(HRESULT result, IDefStatus* status);
HRESULT ComputeHResult(HRESULT result, IDefStatusEx* status);

class DefStatus final : public IDefStatus
{
public:
    DefStatus() = default;
    ~DefStatus() override = default;

    [[nodiscard]] HRESULT GetWhat() const override { return _status.what; }

    [[nodiscard]] const wchar_t* GetWhere() const override { return _status.where; }

    [[nodiscard]] const wchar_t* GetDesc() const override { return _status.desc; }

    [[nodiscard]] std::uint32_t GetLine() const override { return _status.line; }

    [[nodiscard]] const DEFSTATUS* GetDefStatus() const override { return &_status; }

    [[nodiscard]] bool Failed() const override { return _status.what < 0; }

    [[nodiscard]] bool Succeeded() const override { return _status.what >= 0; }

    [[nodiscard]] HRESULT GetHResult();

    bool SetError(HRESULT error, const wchar_t* where, std::uint32_t line, const wchar_t* description, std::uint32_t descriptionLength)
        override;

private:
    DEFSTATUS _status {};
};

class DefStatusWrapper : public IDefStatus
{
public:
    explicit DefStatusWrapper(DEFSTATUS* status);
    ~DefStatusWrapper() override = default;

    virtual void Init();
    [[nodiscard]] HRESULT GetWhat() const override;
    [[nodiscard]] const wchar_t* GetWhere() const override;
    [[nodiscard]] const wchar_t* GetDesc() const override;
    [[nodiscard]] std::uint32_t GetLine() const override;
    [[nodiscard]] const DEFSTATUS* GetDefStatus() const override;
    [[nodiscard]] bool Failed() const override;
    [[nodiscard]] bool Succeeded() const override;
    virtual void Reset();
    bool SetError(HRESULT error, const wchar_t* where, std::uint32_t line, const wchar_t* description, std::uint32_t descriptionLength)
        override;
    [[nodiscard]] virtual HRESULT GetHResult();
    virtual bool TryAddSpecificErrorCode(HRESULT error);

protected:
    DEFSTATUS* m_status;
};

class DefStatusEx final : public IDefStatusEx
{
public:
    DefStatusEx() = default;
    ~DefStatusEx() override = default;

    [[nodiscard]] HRESULT GetWhat() const override { return _status.what; }

    [[nodiscard]] const wchar_t* GetWhere() const override { return _status.where; }

    [[nodiscard]] const wchar_t* GetDesc() const override { return _status.desc; }

    [[nodiscard]] std::uint32_t GetLine() const override { return _status.line; }

    [[nodiscard]] const DEFSTATUS* GetDefStatus() const override { return &_status; }

    [[nodiscard]] bool Succeeded() const override { return _status.what >= 0; }

    [[nodiscard]] bool Failed() const override { return _status.what < 0; }

    [[nodiscard]] HRESULT GetHResult() override;

    [[nodiscard]] const std::vector<const IDefStatus*>& GetWarningList() const override { return m_warningView; }

    void Init();
    void Reset() override;
    void ResetErrorLocation() override;
    void SetErrorLocation(const wchar_t* location) override;

    bool SetError(HRESULT error, const wchar_t* where, std::uint32_t line, const wchar_t* description, std::uint32_t descriptionLength)
        override;
    bool SetError(HRESULT error, const wchar_t* where, std::uint32_t line, const wchar_t* description) override;
    bool SetError(HRESULT error, const wchar_t* description) override;

    bool AddWarning(HRESULT warning, const wchar_t* where, std::uint32_t line, const wchar_t* description, std::uint32_t descriptionLength)
        override;
    bool AddWarning(HRESULT warning, const wchar_t* where, std::uint32_t line, const wchar_t* description) override;
    bool AddWarning(HRESULT warning, const wchar_t* description) override;
    bool TryAddSpecificErrorCode(HRESULT error);

    bool DiagnosticLogA(const char* format, ...) override;
    bool DiagnosticLogWithPrefixA(const char* prefix, const char* format, ...) override;
    bool DiagnosticLogWithErrorCodeA(const char* message, HRESULT error) override;

    void SetDiagnosticLoggingEnabled(bool enabled);

private:
    const wchar_t* _AddStringToStore(const wchar_t* value);

    DEFSTATUS _status {};
    std::vector<std::unique_ptr<DefStatus>> m_warnings;
    std::vector<const IDefStatus*> m_warningView;
    std::list<std::wstring> m_stringStore;
    const wchar_t* m_errorLocation {};
    std::uint32_t m_diagnosticFlags {};
};

} // namespace Microsoft::Resources
