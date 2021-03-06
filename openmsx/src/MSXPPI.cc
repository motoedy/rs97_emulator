// $Id: MSXPPI.cc 12622 2012-06-14 20:11:33Z m9710797 $

#include "MSXPPI.hh"
#include "I8255.hh"
#include "Keyboard.hh"
#include "LedStatus.hh"
#include "MSXCPUInterface.hh"
#include "MSXMotherBoard.hh"
#include "Reactor.hh"
#include "KeyClick.hh"
#include "CassettePort.hh"
#include "RenShaTurbo.hh"
#include "serialize.hh"
#include "unreachable.hh"
#include <string>

namespace openmsx {

// MSXDevice

static Keyboard* createKeyboard(const DeviceConfig& config)
{
	bool keyGhosting = config.getChildDataAsBool("key_ghosting", true);
	bool keyGhostingSGCprotected =
		config.getChildDataAsBool("key_ghosting_sgc_protected", true);
	string_ref keyboardType = config.getChildData("keyboard_type", "int");
	bool hasKeypad = config.getChildDataAsBool("has_keypad", true);
	bool hasYesNoKeys = config.getChildDataAsBool("has_yesno_keys", false);
	bool codeKanaLocks = config.getChildDataAsBool("code_kana_locks", false);
	bool graphLocks = config.getChildDataAsBool("graph_locks", false);
	MSXMotherBoard& motherBoard = config.getMotherBoard();
	return new Keyboard(motherBoard,
	                    motherBoard.getScheduler(),
	                    motherBoard.getCommandController(),
	                    motherBoard.getReactor().getEventDistributor(),
	                    motherBoard.getMSXEventDistributor(),
	                    motherBoard.getStateChangeDistributor(),
	                    keyboardType, hasKeypad, hasYesNoKeys,
	                    keyGhosting, keyGhostingSGCprotected,
	                    codeKanaLocks, graphLocks);
}

MSXPPI::MSXPPI(const DeviceConfig& config)
	: MSXDevice(config)
	, cassettePort(getMotherBoard().getCassettePort())
	, renshaTurbo(getMotherBoard().getRenShaTurbo())
	, i8255(new I8255(*this, getCurrentTime(), getCliComm()))
	, click(new KeyClick(config))
	, keyboard(createKeyboard(config))
	, prevBits(15)
	, selectedRow(0)
{
	reset(getCurrentTime());
}

MSXPPI::~MSXPPI()
{
	powerDown(EmuTime::dummy());
}

void MSXPPI::reset(EmuTime::param time)
{
	i8255->reset(time);
	click->reset(time);
}

void MSXPPI::powerDown(EmuTime::param /*time*/)
{
	getLedStatus().setLed(LedStatus::CAPS, false);
}

byte MSXPPI::readIO(word port, EmuTime::param time)
{
	switch (port & 0x03) {
	case 0:
		return i8255->readPortA(time);
	case 1:
		return i8255->readPortB(time);
	case 2:
		return i8255->readPortC(time);
	case 3:
		return i8255->readControlPort(time);
	default: // unreachable, avoid warning
		UNREACHABLE;
		return 0;
	}
}

byte MSXPPI::peekIO(word port, EmuTime::param time) const
{
	switch (port & 0x03) {
	case 0:
		return i8255->peekPortA(time);
	case 1:
		return i8255->peekPortB(time);
	case 2:
		return i8255->peekPortC(time);
	case 3:
		return i8255->readControlPort(time);
	default: // unreachable, avoid warning
		UNREACHABLE;
		return 0;
	}
}

void MSXPPI::writeIO(word port, byte value, EmuTime::param time)
{
	switch (port & 0x03) {
	case 0:
		i8255->writePortA(value, time);
		break;
	case 1:
		i8255->writePortB(value, time);
		break;
	case 2:
		i8255->writePortC(value, time);
		break;
	case 3:
		i8255->writeControlPort(value, time);
		break;
	default:
		UNREACHABLE;
	}
}


// I8255Interface

byte MSXPPI::readA(EmuTime::param time)
{
	return peekA(time);
}
byte MSXPPI::peekA(EmuTime::param /*time*/) const
{
	// port A is normally an output on MSX, reading from an output port
	// is handled internally in the 8255
	// TODO check this on a real MSX
	// TODO returning 0 fixes the 'get_selected_slot' script right after
	//      reset (when PPI directions are not yet set). For now this
	//      solution is good enough.
	return 0;
}
void MSXPPI::writeA(byte value, EmuTime::param /*time*/)
{
	getCPUInterface().setPrimarySlots(value);
}

byte MSXPPI::readB(EmuTime::param time)
{
	return peekB(time);
}
byte MSXPPI::peekB(EmuTime::param time) const
{
       if (selectedRow != 8) {
               return keyboard->getKeys()[selectedRow];
       } else {
               return keyboard->getKeys()[8] | (renshaTurbo.getSignal(time) ? 1:0);
       }
}
void MSXPPI::writeB(byte /*value*/, EmuTime::param /*time*/)
{
	// probably nothing happens on a real MSX
}

nibble MSXPPI::readC1(EmuTime::param time)
{
	return peekC1(time);
}
nibble MSXPPI::peekC1(EmuTime::param /*time*/) const
{
	return 15; // TODO check this
}
nibble MSXPPI::readC0(EmuTime::param time)
{
	return peekC0(time);
}
nibble MSXPPI::peekC0(EmuTime::param /*time*/) const
{
	return 15; // TODO check this
}
void MSXPPI::writeC1(nibble value, EmuTime::param time)
{
	if ((prevBits ^ value) & 1) {
		cassettePort.setMotor((value & 1) == 0, time); // 0=0n, 1=Off
	}
	if ((prevBits ^ value) & 2) {
		cassettePort.cassetteOut((value & 2) != 0, time);
	}
	if ((prevBits ^ value) & 4) {
		getLedStatus().setLed(LedStatus::CAPS, (value & 4) == 0);
	}
	if ((prevBits ^ value) & 8) {
		click->setClick((value & 8) != 0, time);
	}
	prevBits = value;
}
void MSXPPI::writeC0(nibble value, EmuTime::param /*time*/)
{
	selectedRow = value;
}


template<typename Archive>
void MSXPPI::serialize(Archive& ar, unsigned /*version*/)
{
	ar.template serializeBase<MSXDevice>(*this);
	ar.serialize("i8255", *i8255);

	// merge prevBits and selectedRow into one byte
	byte portC = (prevBits << 4) | (selectedRow << 0);
	ar.serialize("portC", portC);
	if (ar.isLoader()) {
		selectedRow = (portC >> 0) & 0xF;
		nibble bits = (portC >> 4) & 0xF;
		writeC1(bits, getCurrentTime());
	}
	ar.serialize("keyboard", *keyboard);
}
INSTANTIATE_SERIALIZE_METHODS(MSXPPI);
REGISTER_MSXDEVICE(MSXPPI, "PPI");

} // namespace openmsx
