#include "MIDI\CookedMidi.h"
#include "MIDI\Enums.h"
#include "MIDI\Parser\MTrkEvent.h"
#include "MIDI\Util.h"
#include <algorithm>
#include <array>
#include <cassert>

#ifndef DISTR
#include "Windows\framework.h"
#include <sstream>
#endif // !DISTR


namespace MIDI {
CookedMidi::TempoMap::TempoMap() {
	_map[0] = 500000;
}
std::pair<i64, u32> CookedMidi::TempoMap::operator[](i64 timeMicros) const {
	if (const auto it = GetIt(timeMicros); it != _map.end()) {
		return *it;
	}
	return { 0, 0 };
}

std::map<i64, u32>::const_iterator CookedMidi::TempoMap::GetIt(i64 timeMicros) const {
	const auto above = _map.upper_bound(timeMicros);
	if (above == _map.begin()) {
		return above;
	} else return std::prev(above);
}

std::pair<i64, u32> CookedMidi::TempoMap::next(i64 timeMicros) const {
	if (const auto it = _map.upper_bound(timeMicros); it != _map.end()) {
		return *it;
	}
	return { 0, 0 };
}

CookedMidi::CookedMidi() {}
CookedMidi::CookedMidi(const RawMidi& raw) {
	Cook(raw);
}

void CookedMidi::Cook(const RawMidi& raw) {
	header = raw.header;
	tracks.clear();
	
	CookTempoMap(raw);

	std::array<std::map<u32, Note>, 16> activeNotes;

	for (const auto& trackChunk : raw.tracks) {
		tracks.emplace_back();
		Track& track = tracks.back();
		u32 currentTick = 0;
		u64 currentTime = 0;

		for (const auto& pEvent : trackChunk.events) {
			currentTick += pEvent->delta;
			// TODO: SMPTE
			currentTime += pEvent->delta * tempoMap[currentTime].second / header.division.ticksPerQuarter;

			const u8 eventType = pEvent->GetEventType();
			if (eventType == 0xff) {
				const MetaEvent* pMetaEvent = reinterpret_cast<const MetaEvent*>(pEvent.get());
				if (pMetaEvent->type == MIDI_META_EVENT_TRACK_NAME) {
					track.name = std::string(reinterpret_cast<char*>(pMetaEvent->data), pMetaEvent->length);
				}
			} else {
				if (eventType >> 4 == MIDI_EVENT_STATUS_NOTE_ON) {
					const NoteEvent* pNoteEvent = reinterpret_cast<const NoteEvent*>(pEvent.get());
					if (pNoteEvent->vel() == 0) goto label_note_off;
					track.highestNote = std::max(track.highestNote, pNoteEvent->key());
					track.lowestNote = std::min(track.lowestNote, pNoteEvent->key());
					std::map<u32, Note>& chActive = activeNotes[pNoteEvent->Channel()];
					if (auto it = chActive.find(pNoteEvent->key()); it != chActive.end()) {
						Note& oldNote = it->second;
						oldNote.lengthTicks = currentTick - oldNote.startTick;
						oldNote.lengthMicro = currentTime - oldNote.startMicro;
						track.notes/*[oldNote.channel]*/.emplace_back(oldNote);
					}
					chActive[pNoteEvent->key()] = Note{
						.startTick = currentTick,
						.lengthTicks = 0,
						.startMicro = currentTime,
						.lengthMicro = 0,
						.pitch = pNoteEvent->key(),
						.velocity = pNoteEvent->vel(),
						.channel = pNoteEvent->Channel()
					};
				} else if (eventType >> 4 == MIDI_EVENT_STATUS_NOTE_OFF) {
				label_note_off:
					const NoteEvent* pNoteEvent = reinterpret_cast<const NoteEvent*>(pEvent.get());
					std::map<u32, Note>& chActive = activeNotes[pNoteEvent->Channel()];
					if (auto it = chActive.find(pNoteEvent->key()); it != chActive.end()) {
						Note& oldNote = it->second;
						oldNote.lengthTicks = currentTick - oldNote.startTick;
						oldNote.lengthMicro = currentTime - oldNote.startMicro;
						track.notes/*[oldNote.channel]*/.emplace_back(oldNote);
						chActive.erase(pNoteEvent->key());
					}
				}
			}
		}

		#ifndef DISTR
		for (usize i = 0; i < activeNotes.size(); i++) {
			const auto& chActive = activeNotes[i];
			if (!chActive.empty()) {
				std::ostringstream oss;
				oss << std::format("Channel {:d} has ended with {:d} active notes:\n", i, chActive.size());
				for (const auto& p : chActive) {
					oss << "    " << std::format("pitch: {:d}  startTick: {:d}", p.first, p.second.startTick) << "\n";
				}
				DEBUG_LOG(oss.view().data());
			}
		}
		#endif // !DISTR

		std::sort(track.notes.begin(), track.notes.end(), [](const Note& n1, const Note& n2) { return n1.startMicro < n2.startMicro; });
	}
}

void CookedMidi::CookTempoMap(const RawMidi& raw) {
	tempoMap._map.clear();
	tempoMap._map[0] = 500000;

	std::map<i64, std::pair<i64, u32>> tempoMapByTick; // <tick, <delta, tempo micros>>

	for (const auto& trackChunk : raw.tracks) {
		i64 currentTick = 0;
		i64 lastTempoTick = 0;
		for (const auto& pEvent : trackChunk.events) {
			currentTick += pEvent->delta;

			const u8 eventType = pEvent->GetEventType();
			if (eventType == 0xff) {
				const MetaEvent* pMetaEvent = reinterpret_cast<const MetaEvent*>(pEvent.get());
				if (pMetaEvent->type == MIDI_META_EVENT_TEMPO) {
					tempoMapByTick[currentTick] = { currentTick - lastTempoTick, util::tempo_micros(pMetaEvent->data) };
					lastTempoTick = currentTick;
				}
			}
		}
	}

	i64 currentTime = 0;
	u32 currentTempo = 500000;
	auto div = header.division;
	if (div.fmt == MIDI_DIVISION_FORMAT_TPQN) {
		for (const auto& p : tempoMapByTick) {
			currentTime += currentTempo * p.second.first / div.ticksPerQuarter;
			currentTempo = p.second.second;
			tempoMap._map[currentTime] = currentTempo;
		}
	} else { //MIDI_DIVISION_FORMAT_SMPTE
		for (const auto& p : tempoMapByTick) {
			currentTime += 1'000'000 * p.second.first / (-div.smpte * div.ticksPerFrame);
			currentTempo = p.second.second;
			tempoMap._map[currentTime] = currentTempo;
		}
	}
}

}