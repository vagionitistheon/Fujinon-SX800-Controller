#include "ProtocolParser.h"
#include "OpticalTables.h"

#include <cctype>
#include <glog/logging.h>

namespace FujinonSX800 {

bool ProtocolParser::parsePacket(
    const std::vector<std::uint8_t>& packet,
    CameraStatus& status,
    std::string_view lastQuery) noexcept
{
    if (!PelcoDFrame::isValidFrame(packet)) {
        LOG(WARNING) << "ProtocolParser rejected invalid frame (checksum or format failure)";
        return false;
    }

    VLOG(2) << "Parsing packet (" << packet.size() << " bytes), last query: '" << lastQuery << "'";

    if (packet.size() == PelcoDFrame::GeneralResponseSize) {
        return parseGeneralAck(packet, status);
    }
    if (packet.size() == PelcoDFrame::StandardFrameSize) {
        return parseExtendedResponse(packet, status);
    }
    if (packet.size() == PelcoDFrame::QueryResponseSize) {
        return parseQueryResponse(packet, status, lastQuery);
    }

    LOG(WARNING) << "Unhandled Pelco-D packet size: " << packet.size();
    return false;
}

bool ProtocolParser::parseGeneralAck(
    const std::vector<std::uint8_t>& packet, CameraStatus& status) noexcept
{
    status.address = packet[1];
    status.rs485Address = packet[1];
    status.connected = true;
    status.isConnected = true;
    return true;
}

bool ProtocolParser::parseExtendedResponse(
    const std::vector<std::uint8_t>& packet, CameraStatus& status) noexcept
{
    status.address = packet[1];
    status.rs485Address = packet[1];
    status.connected = true;
    status.isConnected = true;

    const std::uint8_t cmd2 { packet[3] };
    const std::uint8_t d1 { packet[4] };
    const std::uint8_t d2 { packet[5] };

    switch (cmd2) {
    case 0x5DU: { // Zoom position response
        status.zoomPosition = static_cast<std::uint16_t>((d1 << 8U) | d2);
        status.focalLengthMm = OpticalTables::pulseToFocalLength(status.zoomPosition);
        status.horizontalFovDeg = OpticalTables::pulseToHorizontalFov(status.zoomPosition);
        return true;
    }
    case 0x81U: { // Focus position response
        status.focusPosition = static_cast<std::uint16_t>((d1 << 8U) | d2);
        status.focusDistanceM = OpticalTables::pulseToDistance(status.focusPosition);
        status.subjectDistanceM = status.focusDistanceM;
        return true;
    }
    case 0x8BU: // FW version response
    case 0x7DU: {
        status.fwVersionMajor = d1;
        status.fwVersionMinor = d2;
        return true;
    }
    case 0x8DU: { // Lens status response
        // d1 bit flags
        status.irisCloseLimit = (d1 & 0x01U) != 0U;
        status.irisOpenLimit  = (d1 & 0x02U) != 0U;
        status.irisMoving     = (d1 & 0x04U) != 0U;
        status.focusFarLimit  = (d1 & 0x08U) != 0U;
        status.focusNearLimit = (d1 & 0x10U) != 0U;
        status.focusMoving    = (d1 & 0x20U) != 0U;
        // d2 bit flags
        status.zoomMoving     = (d2 & 0x01U) != 0U;
        status.zoomTeleLimit  = (d2 & 0x02U) != 0U;
        status.zoomWideLimit  = (d2 & 0x04U) != 0U;
        return true;
    }
    case 0x89U: { // Iris position response
        status.irisPosition = static_cast<std::uint16_t>((d1 << 8U) | d2);
        status.manualIrisPosition = status.irisPosition;
        return true;
    }
    case 0xC1U: // Log Data / Temperature response
    case 0xD1U: {
        if (d1 != 0 || d2 != 0) {
            const auto intPart = static_cast<std::int8_t>(d1);
            status.internalTemperatureC = static_cast<double>(intPart) + (static_cast<double>(d2) / 10.0);
        }
        return true;
    }
    default:
        break;
    }

    return true;
}

bool ProtocolParser::parseQueryResponse(
    const std::vector<std::uint8_t>& packet,
    CameraStatus& status,
    std::string_view lastQuery) noexcept
{
    status.address = packet[1];
    status.rs485Address = packet[1];
    status.connected = true;
    status.isConnected = true;

    // Check if it matches Serial Query or has ASCII characters
    if (lastQuery == "QuerySerial" || lastQuery.empty()) {
        std::string s;
        bool allPrintable { true };
        for (std::size_t i { 2U }; i < 17U; ++i) {
            const char c = static_cast<char>(packet[i]);
            if (c == '\0') {
                break;
            }
            if (std::isprint(static_cast<unsigned char>(c)) == 0) {
                allPrintable = false;
                break;
            }
            s.push_back(c);
        }
        if (allPrintable && !s.empty()) {
            status.serialNumber = s;
            if (lastQuery == "QuerySerial") {
                return true;
            }
        }
    }

    if (lastQuery == "QueryFw") {
        if (packet[2] != 0 || packet[3] != 0) {
            status.fwVersionMajor = packet[2];
            status.fwVersionMinor = packet[3];
            return true;
        }
    }

    if (lastQuery == "QueryTemperature" || lastQuery == "QueryLogData") {
        if (packet[2] != 0 || packet[3] != 0) {
            const auto intPart = static_cast<std::int8_t>(packet[2]);
            status.internalTemperatureC = static_cast<double>(intPart) + (static_cast<double>(packet[3]) / 10.0);
            return true;
        }
    }

    if (lastQuery == "QueryPhotoSettings") {
        status.afArea = static_cast<AfArea>(packet[2]);
        status.afSensitivity = static_cast<AfSensitivity>(packet[3]);
        status.dayNightMode = static_cast<DayNightMode>(packet[4]);
        status.irWavelength = static_cast<IrWavelength>(packet[5]);
        status.opticalStabilization = static_cast<OpticalStabilization>(packet[6]);
        status.stabilizationMode = status.opticalStabilization;
        return true;
    }

    if (lastQuery == "QueryImageQuality") {
        status.vlcFilter = static_cast<VlcFilterMode>(packet[2]);
        status.wdr = static_cast<WdrMode>(packet[3]);
        status.wdrMode = status.wdr;
        status.deHeatHaze = static_cast<DeHeatHazeMode>(packet[4]);
        status.deHeatHazeMode = status.deHeatHaze;
        status.defog = static_cast<DefogMode>(packet[5]);
        status.defogMode = status.defog;
        status.brightness = packet[6];
        status.contrast = packet[7];
        status.saturation = packet[8];
        status.sharpness = packet[9];
        status.whiteBalance = static_cast<WhiteBalanceMode>(packet[10]);
        status.whiteBalanceMode = status.whiteBalance;
        status.colorTemperatureKelvin = static_cast<std::uint16_t>((packet[11] << 8U) | packet[12]);
        status.colorTempKelvin = status.colorTemperatureKelvin;
        status.digitalZoom = static_cast<DigitalZoomMode>(packet[13]);
        status.digitalZoomMode = status.digitalZoom;
        status.noiseReduction = static_cast<NoiseReductionLevel>(packet[14]);
        return true;
    }

    if (lastQuery == "QueryDisplaySettings") {
        status.osdDateTime = packet[2] != 0U;
        status.dateTimeDisplay = status.osdDateTime;
        status.osdTitle = packet[3] != 0U;
        status.titleDisplay = status.osdTitle;
        status.osdId = packet[4] != 0U;
        status.idDisplay = status.osdId;
        status.osdReticle = packet[5] != 0U;
        status.centerPositionDisplay = status.osdReticle;
        status.osdAntialiasing = packet[6] != 0U;
        status.antialiasingEnabled = status.osdAntialiasing;
        return true;
    }

    if (lastQuery == "QueryOperationSettings") {
        status.videoStandard = static_cast<VideoStandard>(packet[2]);
        status.hdFormat = static_cast<HdVideoFormat>(packet[3]);
        status.terminationEnabled = packet[4] != 0U;
        status.rs485Termination = status.terminationEnabled;
        return true;
    }

    if (lastQuery == "QueryFineSettings") {
        status.fineBrightness = static_cast<std::int8_t>(packet[2]);
        status.brightnessFine = status.fineBrightness;
        status.fineContrast = static_cast<std::int8_t>(packet[3]);
        status.contrastFine = status.fineContrast;
        status.fineSaturation = static_cast<std::int8_t>(packet[4]);
        status.saturationFine = status.fineSaturation;
        status.fineSharpness = static_cast<std::int8_t>(packet[5]);
        status.sharpnessFine = status.fineSharpness;
        status.wbRedShift = static_cast<std::int8_t>(packet[6]);
        status.wbShiftRedFine = status.wbRedShift;
        status.wbBlueShift = static_cast<std::int8_t>(packet[7]);
        status.wbShiftBlueFine = status.wbBlueShift;
        return true;
    }

    if (lastQuery == "QueryDayNightEx") {
        status.filterDay = static_cast<OpticalFilter>(packet[2]);
        status.opticalFilterDay = status.filterDay;
        status.filterNight = static_cast<OpticalFilter>(packet[3]);
        status.opticalFilterNight = status.filterNight;
        return true;
    }

    if (lastQuery == "QuerySpeedEx") {
        status.zoomSpeedEx = packet[2];
        status.focusSpeedEx = packet[3];
        return true;
    }

    return true;
}

std::string ProtocolParser::describeFrame(const std::vector<std::uint8_t>& frame, bool isTx)
{
    if (frame.empty()) {
        return "Empty";
    }

    if (frame.size() == 4U) {
        return isTx ? "General Command" : "General ACK (0x00)";
    }

    if (frame.size() == 18U) {
        return isTx ? "18-byte Query" : "18-byte Query Telemetry Response";
    }

    if (frame.size() == 7U) {
        const std::uint8_t cmd1 = frame[2];
        const std::uint8_t cmd2 = frame[3];

        if (cmd1 == 0x00U && cmd2 == 0x20U) return "Zoom Tele";
        if (cmd1 == 0x00U && cmd2 == 0x40U) return "Zoom Wide";
        if (cmd1 == 0x01U && cmd2 == 0x00U) return "Focus Near";
        if (cmd1 == 0x00U && cmd2 == 0x80U) return "Focus Far";
        if (cmd1 == 0x00U && cmd2 == 0x00U) return "Motion Stop";
        if (cmd1 == 0x00U && cmd2 == 0x4FU) return "Set Zoom Position";
        if (cmd1 == 0x00U && cmd2 == 0x8FU) return "Set Focus Position";
        if (cmd1 == 0x00U && cmd2 == 0x55U) return "Query Zoom Position";
        if (cmd1 == 0x00U && cmd2 == 0x5DU) return "Zoom Position Response";
        if (cmd1 == 0x00U && cmd2 == 0x73U) return "Query Focus Position";
        if (cmd1 == 0x00U && cmd2 == 0x81U) return "Focus Position Response";
        if (cmd1 == 0x00U && cmd2 == 0x7DU) return "Query FW Version";
        if (cmd1 == 0x00U && cmd2 == 0x8BU) return "FW Version Response";
        if (cmd1 == 0x00U && cmd2 == 0x7FU) return "Query Lens Status";
        if (cmd1 == 0x00U && cmd2 == 0x8DU) return "Lens Status Response";
        if (cmd1 == 0xF0U && cmd2 == 0x07U) return "One-Push AF";
        if (cmd1 == 0xF0U && cmd2 == 0x21U) return "Set VLC Filter";
        if (cmd1 == 0xF0U && cmd2 == 0x29U) return "Set Defog Mode";
        if (cmd1 == 0xF0U && cmd2 == 0x27U) return "Set De-Heat Haze";
        if (cmd1 == 0xF0U && cmd2 == 0x8BU) return "Virtual Menu Key";

        char buf[32];
        std::snprintf(buf, sizeof(buf), "Cmd [0x%02X, 0x%02X]", cmd1, cmd2);
        return std::string(buf);
    }

    return "Unknown Packet";
}

} // namespace FujinonSX800
