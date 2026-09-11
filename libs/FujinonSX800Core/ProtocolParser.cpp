#include "ProtocolParser.h"
#include "OpticalTables.h"

#include <cctype>
#include <glog/logging.h>

namespace FujinonSX800 {

bool ProtocolParser::parsePacket(
    const std::vector<std::uint8_t>& packet, CameraStatus& status, std::string_view lastQuery) noexcept
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

bool ProtocolParser::parseGeneralAck(const std::vector<std::uint8_t>& packet, CameraStatus& status) noexcept
{
    status.address = packet[1];
    status.rs485Address = packet[1];
    status.connected = true;
    status.isConnected = true;
    return true;
}

bool ProtocolParser::parseExtendedResponse(const std::vector<std::uint8_t>& packet, CameraStatus& status) noexcept
{
    status.address = packet[1];
    status.rs485Address = packet[1];
    status.connected = true;
    status.isConnected = true;

    const std::uint8_t resp1 { packet[2] };
    const std::uint8_t resp2 { packet[3] };
    const std::uint8_t d1 { packet[4] };
    const std::uint8_t d2 { packet[5] };

    // Standard & FF Extended Commands (RESP1 == 0x00)
    if (resp1 == 0x00U) {
        switch (resp2) {
        case 0x83U: // Fujinon alternative zoom position opcode
        case 0x5DU: { // Zoom position response
            status.zoomPosition = static_cast<std::uint16_t>((d1 << 8U) | d2);
            status.focalLengthMm = OpticalTables::pulseToFocalLength(status.zoomPosition);
            status.horizontalFovDeg = OpticalTables::pulseToHorizontalFov(status.zoomPosition);
            status.verticalFovDeg = OpticalTables::pulseToVerticalFov(status.zoomPosition);
            status.diagonalFovDeg = OpticalTables::pulseToDiagonalFov(status.zoomPosition);
            status.ifovMrad = OpticalTables::calculateIfovMrad(status.focalLengthMm);
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
            status.irisCloseLimit = (d1 & 0x01U) != 0U;
            status.irisOpenLimit = (d1 & 0x02U) != 0U;
            status.irisMoving = (d1 & 0x04U) != 0U;
            status.focusFarLimit = (d1 & 0x08U) != 0U;
            status.focusNearLimit = (d1 & 0x10U) != 0U;
            status.focusMoving = (d1 & 0x20U) != 0U;
            status.zoomMoving = (d2 & 0x01U) != 0U;
            status.zoomTeleLimit = (d2 & 0x02U) != 0U;
            status.zoomWideLimit = (d2 & 0x04U) != 0U;
            return true;
        }
        case 0x91U: { // Manual Iris position / F-number response
            status.manualIrisFNo = static_cast<ManualIrisFNo>(d2 != 0U ? d2 : d1);
            status.irisPosition = static_cast<std::uint16_t>((d1 << 8U) | d2);
            status.manualIrisPosition = status.irisPosition;
            return true;
        }
        case 0x97U: { // Manual Shutter Speed response
            status.shutterSpeed = static_cast<ShutterSpeed>(d1);
            status.manualShutterSpeed = d1;
            return true;
        }
        case 0x9BU: { // Manual ISO response
            status.isoSensitivity = static_cast<IsoSensitivity>(d1);
            status.manualIso = d1;
            return true;
        }
        case 0x9DU: { // Manual Iris response
            status.manualIrisFNo = static_cast<ManualIrisFNo>(d1);
            return true;
        }
        case 0xAFU: { // Manual settings extended query response (5.3.23)
            switch (d1) {
            case 0x25U:
                status.zoomSpeed = d2;
                status.zoomSpeedEx = d2;
                break;
            case 0x27U:
                status.focusSpeed = d2;
                status.focusSpeedEx = d2;
                break;
            case 0x2BU:
                status.autoFocusMode = (d2 == 0x00U) ? AutoFocusMode::On : AutoFocusMode::Off;
                break;
            case 0x2DU:
                status.autoIrisMode = (d2 == 0x02U) ? AutoIrisMode::On : AutoIrisMode::Off;
                break;
            case 0x2FU:
                status.agcMode = (d2 == 0x02U) ? AgcMode::Auto : AgcMode::Manual;
                break;
            case 0x31U:
                status.blcMode = (d2 == 0x02U) ? BlcMode::On : BlcMode::Off;
                break;
            default:
                break;
            }
            return true;
        }
        default:
            break;
        }
    }

    // Original Commands 1 - 8 (RESP1 == 0xF0)
    if (resp1 == 0xF0U) {
        switch (resp2) {
        case 0x1FU: { // Photo settings query response (5.4.14)
            switch (d1) {
            case 0x03U:
            case 0x09U: // Legacy compatibility
                status.afArea = static_cast<AfArea>(d2);
                break;
            case 0x05U:
            case 0x0BU: // Legacy compatibility
                status.afSensitivity = static_cast<AfSensitivity>(d2);
                break;
            case 0x0DU:
                status.dayToNightThreshold = d2;
                break;
            case 0x0FU:
                status.dayNightMode = static_cast<DayNightMode>(d2);
                break;
            case 0x11U:
                status.irWavelength = static_cast<IrWavelength>(d2);
                break;
            case 0x13U:
                status.opticalStabilization = static_cast<OpticalStabilization>(d2);
                status.stabilizationMode = status.opticalStabilization;
                break;
            default:
                break;
            }
            return true;
        }
        case 0x3FU: { // Image quality query response (5.5.15)
            switch (d1) {
            case 0x21U:
                status.vlcFilter = static_cast<VlcFilterMode>(d2);
                break;
            case 0x23U:
            case 0x25U: // Legacy compatibility
                status.wdr = static_cast<WdrMode>(d2);
                status.wdrMode = status.wdr;
                break;
            case 0x27U:
                status.deHeatHaze = static_cast<DeHeatHazeMode>(d2);
                status.deHeatHazeMode = status.deHeatHaze;
                break;
            case 0x29U:
                status.defog = static_cast<DefogMode>(d2);
                status.defogMode = status.defog;
                break;
            case 0x2BU:
                status.brightness = d2;
                break;
            case 0x2DU:
                status.contrast = d2;
                break;
            case 0x2FU:
                status.saturation = d2;
                break;
            case 0x31U:
                status.sharpness = d2;
                break;
            case 0x33U:
                status.colorTempKelvin = static_cast<std::uint16_t>((d1 << 8U) | d2);
                status.colorTemperatureKelvin = status.colorTempKelvin;
                break;
            case 0x35U:
                status.whiteBalance = static_cast<WhiteBalanceMode>(d2);
                status.whiteBalanceMode = status.whiteBalance;
                break;
            case 0x37U:
                status.digitalZoom = static_cast<DigitalZoomMode>(d2);
                status.digitalZoomMode = status.digitalZoom;
                break;
            case 0x39U:
                status.noiseReduction = static_cast<NoiseReductionLevel>(d2);
                break;
            default:
                break;
            }
            return true;
        }
        case 0x5FU: { // Display settings query response (5.6.15)
            switch (d1) {
            case 0x43U:
                status.dateTimeDisplay = (d2 != 0U);
                status.osdDateTime = status.dateTimeDisplay;
                break;
            case 0x45U:
                status.dateTimePosition = static_cast<OsdPosition>(d2);
                break;
            case 0x47U:
                status.titleDisplay = (d2 != 0U);
                status.osdTitle = status.titleDisplay;
                break;
            case 0x4BU:
                status.titlePosition = static_cast<OsdPosition>(d2);
                break;
            case 0x4DU:
                status.idDisplay = (d2 != 0U);
                status.osdId = status.idDisplay;
                break;
            case 0x4FU:
                status.idPosition = static_cast<OsdPosition>(d2);
                break;
            case 0x51U:
                status.centerPositionDisplay = (d2 != 0U);
                status.osdReticle = status.centerPositionDisplay;
                break;
            case 0x55U:
                status.antialiasingEnabled = (d2 != 0U);
                status.osdAntialiasing = status.antialiasingEnabled;
                break;
            default:
                break;
            }
            return true;
        }
        case 0x8FU: { // Operation settings query response (5.7.23)
            switch (d1) {
            case 0x61U:
                status.timeDisplayFormat = static_cast<TimeDisplayFormat>(d2);
                break;
            case 0x63U:
                status.dateDisplayFormat = static_cast<DateDisplayFormat>(d2);
                break;
            case 0x67U:
                status.videoStandard = static_cast<VideoStandard>(d2);
                break;
            case 0x69U:
                status.hdFormat = static_cast<HdFormat>(d2);
                break;
            case 0x6BU:
                status.videoDisplayMode = static_cast<VideoDisplayMode>(d2);
                break;
            case 0x6FU:
                status.rs485Address = d2;
                status.address = d2;
                break;
            case 0x73U:
                status.rs485Termination = (d2 != 0U);
                status.terminationEnabled = status.rs485Termination;
                break;
            case 0x77U:
                status.sdRecordMode = static_cast<SdRecordMode>(d2);
                break;
            case 0x85U:
                status.language = static_cast<Language>(d2);
                break;
            default:
                break;
            }
            return true;
        }
        case 0xAFU: { // Key settings query response (5.8.15)
            if (d1 == 0x9BU) {
                status.sdPlaybackMode = static_cast<SdPlaybackMode>(d2);
            }
            return true;
        }
        case 0xB3U: { // SD movie count (5.9.1)
            status.sdMovieCount = static_cast<std::uint16_t>((d1 << 8U) | d2);
            return true;
        }
        case 0xD3U: { // Log data response (5.10.1)
            return true;
        }
        case 0xFFU: { // Fine settings query response (5.11.15)
            switch (d1) {
            case 0xEBU:
                status.fineBrightness = static_cast<std::int8_t>(d2);
                status.brightnessFine = status.fineBrightness;
                break;
            case 0xEDU:
                status.fineContrast = static_cast<std::int8_t>(d2);
                status.contrastFine = status.fineContrast;
                break;
            case 0xEFU:
                status.fineSaturation = static_cast<std::int8_t>(d2);
                status.saturationFine = status.fineSaturation;
                break;
            case 0xF1U:
                status.fineSharpness = static_cast<std::int8_t>(d2);
                status.sharpnessFine = status.fineSharpness;
                break;
            case 0xF5U:
                status.wbRedShift = static_cast<std::int8_t>(d2);
                status.wbShiftRedFine = status.wbRedShift;
                break;
            case 0xF7U:
                status.wbBlueShift = static_cast<std::int8_t>(d2);
                status.wbShiftBlueFine = status.wbBlueShift;
                break;
            default:
                break;
            }
            return true;
        }
        case 0xC1U: { // Temperature query response
            const double whole = static_cast<double>(static_cast<std::int8_t>(d1));
            const double frac = static_cast<double>(d2) / 10.0;
            status.internalTemperatureC = (whole >= 0.0) ? (whole + frac) : (whole - frac);
            status.temperatureValid = true;
            return true;
        }
        default:
            break;
        }
    }

    // Original Commands 9 & 10 (RESP1 == 0xF1)
    if (resp1 == 0xF1U) {
        if (resp2 == 0x1FU) { // Day/Night Ex query response (5.12.10)
            switch (d1) {
            case 0x01U:
                status.dayNightMode = static_cast<DayNightMode>(d2);
                break;
            case 0x03U:
                status.dayToNightThreshold = d2;
                break;
            case 0x05U:
                status.nightToDayThreshold = d2;
                break;
            case 0x07U:
                status.dayNightAutoDelaySec = d2;
                break;
            case 0x0DU:
                status.opticalFilterDay = static_cast<OpticalFilter>(d2);
                status.filterDay = status.opticalFilterDay;
                break;
            case 0x0FU:
                status.opticalFilterNight = static_cast<OpticalFilter>(d2);
                status.filterNight = status.opticalFilterNight;
                break;
            default:
                break;
            }
            return true;
        }
        if (resp2 == 0x2FU) { // Zoom/Focus Speed Ex query response (5.13.4)
            switch (d1) {
            case 0x21U:
                status.digitalZoomMag = d2;
                break;
            case 0x25U:
                status.zoomSpeedEx = d2;
                status.zoomSpeed = d2;
                break;
            case 0x27U:
                status.focusSpeedEx = d2;
                status.focusSpeed = d2;
                break;
            default:
                break;
            }
            return true;
        }
    }

    return true;
}

bool ProtocolParser::parseQueryResponse(
    const std::vector<std::uint8_t>& packet, CameraStatus& status, std::string_view lastQuery) noexcept
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

        if (cmd1 == 0x00U && cmd2 == 0x20U)
            return "Zoom Tele";
        if (cmd1 == 0x00U && cmd2 == 0x40U)
            return "Zoom Wide";
        if (cmd1 == 0x01U && cmd2 == 0x00U)
            return "Focus Near";
        if (cmd1 == 0x00U && cmd2 == 0x80U)
            return "Focus Far";
        if (cmd1 == 0x00U && cmd2 == 0x00U)
            return "Motion Stop";
        if (cmd1 == 0x00U && cmd2 == 0x4FU)
            return "Set Zoom Position";
        if (cmd1 == 0x00U && cmd2 == 0x8FU)
            return "Set Focus Position";
        if (cmd1 == 0x00U && cmd2 == 0x55U)
            return "Query Zoom Position";
        if (cmd1 == 0x00U && cmd2 == 0x5DU)
            return "Zoom Position Response";
        if (cmd1 == 0x00U && cmd2 == 0x73U)
            return "Query Focus Position";
        if (cmd1 == 0x00U && cmd2 == 0x81U)
            return "Focus Position Response";
        if (cmd1 == 0x00U && cmd2 == 0x7DU)
            return "Query FW Version";
        if (cmd1 == 0x00U && cmd2 == 0x8BU)
            return "FW Version Response";
        if (cmd1 == 0x00U && cmd2 == 0x7FU)
            return "Query Lens Status";
        if (cmd1 == 0x00U && cmd2 == 0x8DU)
            return "Lens Status Response";
        if (cmd1 == 0xF0U && cmd2 == 0x07U)
            return "One-Push AF";
        if (cmd1 == 0xF0U && cmd2 == 0x21U)
            return "Set VLC Filter";
        if (cmd1 == 0xF0U && cmd2 == 0x29U)
            return "Set Defog Mode";
        if (cmd1 == 0xF0U && cmd2 == 0x27U)
            return "Set De-Heat Haze";
        if (cmd1 == 0xF0U && cmd2 == 0x8BU)
            return "Virtual Menu Key";

        char buf[32];
        std::snprintf(buf, sizeof(buf), "Cmd [0x%02X, 0x%02X]", cmd1, cmd2);
        return std::string(buf);
    }

    return "Unknown Packet";
}

} // namespace FujinonSX800
