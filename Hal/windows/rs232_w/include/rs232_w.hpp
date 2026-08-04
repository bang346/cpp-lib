#pragma once

#include <Windows.h>

#include <cstdint>
#include <stdexcept>
#include <string>

#include "bus_if.hpp"

/// @brief
/// @note       Optimized for high data rates. If the data
///             rate is low, check the latency in the FTDI chip.
class windows_uart final : public bus_if
{
public:
    explicit windows_uart(const std::string &port,
                          DWORD baudrate = CBR_115200)
    {
        open(port, baudrate);
    }

    ~windows_uart() override
    {
        close();
    }

    windows_uart(const windows_uart &) = delete;
    windows_uart &operator=(const windows_uart &) = delete;

    int transmit(const std::uint8_t *const data,
                 const std::uint8_t len) const override
    {
        if (!isOpen() || (data == nullptr && len > 0))
        {
            return -1;
        }

        DWORD bytesWritten = 0;

        const BOOL result = WriteFile(
            handle_,
            data,
            static_cast<DWORD>(len),
            &bytesWritten,
            nullptr);

        if (result == FALSE)
        {
            return -2;
        }

        if (bytesWritten != static_cast<DWORD>(len))
        {
            return -3;
        }

        return 0;
    }

    int receive(std::uint8_t *const data,
                const std::uint8_t len) const override
    {
        if (!isOpen() || (data == nullptr && len > 0))
        {
            return -1;
        }

        DWORD totalBytesRead = 0;

        while (totalBytesRead < static_cast<DWORD>(len))
        {
            DWORD bytesRead = 0;

            const BOOL result = ReadFile(
                handle_,
                data + totalBytesRead,
                static_cast<DWORD>(len) - totalBytesRead,
                &bytesRead,
                nullptr);

            if (result == FALSE)
            {
                return -2;
            }

            // Timeout, bevor alle angeforderten Bytes empfangen wurden.
            if (bytesRead == 0)
            {
                return totalBytesRead;
            }

            totalBytesRead += bytesRead;
        }

        return totalBytesRead;
    }

    int transmitreceive(std::uint8_t *const data_tx,
                        std::uint8_t *data_rx,
                        const std::uint8_t len) const override
    {
        const int transmitResult = transmit(data_tx, len);

        if (transmitResult != 0)
        {
            return transmitResult;
        }

        return receive(data_rx, len);
    }

    [[nodiscard]]
    bool isOpen() const noexcept
    {
        return handle_ != INVALID_HANDLE_VALUE;
    }

private:
    void open(const std::string &port, const DWORD baudrate)
    {
        // Auch für COM10 und höher geeignet.
        std::string devicePath = port;

        if (devicePath.rfind(R"(\\.\)", 0) != 0)
        {
            devicePath = R"(\\.\)" + devicePath;
        }

        handle_ = CreateFileA(
            devicePath.c_str(),
            GENERIC_READ | GENERIC_WRITE,
            0,
            nullptr,
            OPEN_EXISTING,
            0,
            nullptr);

        if (handle_ == INVALID_HANDLE_VALUE)
        {
            throw std::runtime_error(
                "Could not open UART port " + port +
                ", Windows error: " + std::to_string(GetLastError()));
        }

        DCB config{};
        config.DCBlength = sizeof(config);

        if (GetCommState(handle_, &config) == FALSE)
        {
            const DWORD error = GetLastError();
            close();

            throw std::runtime_error(
                "GetCommState failed, Windows error: " +
                std::to_string(error));
        }

        config.BaudRate = baudrate;
        config.ByteSize = 8;
        config.Parity = NOPARITY;
        config.StopBits = ONESTOPBIT;

        config.fBinary = TRUE;
        config.fParity = FALSE;
        config.fOutxCtsFlow = FALSE;
        config.fOutxDsrFlow = FALSE;
        config.fDtrControl = DTR_CONTROL_DISABLE;
        config.fDsrSensitivity = FALSE;
        config.fOutX = FALSE;
        config.fInX = FALSE;
        config.fRtsControl = RTS_CONTROL_DISABLE;

        if (SetCommState(handle_, &config) == FALSE)
        {
            const DWORD error = GetLastError();
            close();

            throw std::runtime_error(
                "SetCommState failed, Windows error: " +
                std::to_string(error));
        }

        COMMTIMEOUTS timeouts{};

        // ReadFile wartet maximal ungefähr 100 ms pro Aufruf.
        timeouts.ReadIntervalTimeout = MAXDWORD;
        timeouts.ReadTotalTimeoutMultiplier = MAXDWORD;
        timeouts.ReadTotalTimeoutConstant = 2;
        timeouts.WriteTotalTimeoutMultiplier = 0;
        timeouts.WriteTotalTimeoutConstant = 10;

        if (SetCommTimeouts(handle_, &timeouts) == FALSE)
        {
            const DWORD error = GetLastError();
            close();

            throw std::runtime_error(
                "SetCommTimeouts failed, Windows error: " +
                std::to_string(error));
        }

        // Bereits vorhandene Daten verwerfen.
        PurgeComm(handle_, PURGE_RXCLEAR | PURGE_TXCLEAR);
    }

    void close() noexcept
    {
        if (handle_ != INVALID_HANDLE_VALUE)
        {
            CloseHandle(handle_);
            handle_ = INVALID_HANDLE_VALUE;
        }
    }

    HANDLE handle_{INVALID_HANDLE_VALUE};
};