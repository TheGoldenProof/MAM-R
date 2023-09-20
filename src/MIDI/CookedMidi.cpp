#include "MIDI\CookedMidi.h"
#include "MIDI\Enums.h"
#include "MIDI\Parser\MTrkEvent.h"
#include "MIDI\Util.h"
#include <algorithm>
#include <cassert>

#ifndef DISTR
#include "Windows\framework.h"
#include <sstream>
#endif // !DISTR


namespace MIDI {
CookedMidi::CookedMidi() {}
CookedMidi::CookedMidi(const RawMidi& raw) {
	Cook(raw);
}

void CookedMidi::Cook(const RawMidi& raw) {
	header = raw.header;
	tracks.clear();
	tempoMap.clear();

	std::array<std::map<u32, Note>, 16> activeNotes;

	u32 currentTempo = 500000;
	for (const auto& trackChunk : raw.tracks) {
		tracks.emplace_back();
		Track& track = tracks.back();
		u32 currentTick = 0;
		u64 currentTime = 0;

		for (const auto& pEvent : trackChunk.events) {
			currentTick += pEvent->delta;
			currentTime += pEvent->delta * currentTempo;

			const u8 eventType = pEvent->GetEventType();
			if (eventType == 0xff) {
				const MetaEvent* pMetaEvent = reinterpret_cast<const MetaEvent*>(pEvent.get());
				if (pMetaEvent->type == MIDI_META_EVENT_TEMPO) {
					currentTempo = util::tempo_micros(pMetaEvent->data);
					tempoMap.emplace_back(currentTick, currentTempo);
				}
			} else {
				if (eventType >> 4 == MIDI_EVENT_STATUS_NOTE_ON) {
					const NoteEvent* pNoteEvent = reinterpret_cast<const NoteEvent*>(pEvent.get());
					if (pNoteEvent->vel() == 0) goto label_note_off;
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

}