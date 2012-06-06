#include <set>

#include "EventList.hpp"
#include "../SAL/SALInst.hpp"

namespace fi
{

EventId EventList::add(BaseEvent* ev, ExperimentFlow* pExp)
{
	assert(ev != NULL && "FATAL ERROR: Event (of base type BaseEvent*) cannot be NULL!");
	// a zero counter does not make sense
	assert(ev->getCounter() != 0);
	ev->setParent(pExp); // event is linked to experiment flow

	BPEvent *bp_ev;
	if((bp_ev = dynamic_cast<BPEvent*>(ev)) != NULL)
		m_Bp_cache.add(bp_ev);
	m_BufferList.push_back(ev);
	return (ev->getId());
}

void EventList::remove(BaseEvent* ev)
{
	// possible cases:
	// - ev == 0 -> remove all events
	//   * clear m_BufferList
	//   * copy m_FireList to m_DeleteList
	if (ev == 0) {
		for (bufferlist_t::iterator it = m_BufferList.begin(); it != m_BufferList.end(); it++)
			sal::simulator.onEventDeletion(*it);
		for (firelist_t::iterator it = m_FireList.begin(); it != m_FireList.end(); it++)
			sal::simulator.onEventDeletion(*it);
		m_Bp_cache.clear();
		m_BufferList.clear();
		// all remaining active events must not fire anymore
		m_DeleteList.insert(m_DeleteList.end(), m_FireList.begin(), m_FireList.end());

	// - ev != 0 -> remove single event
	//   * find/remove ev in m_BufferList
	//   * if ev in m_FireList, copy to m_DeleteList
	} else {
		sal::simulator.onEventDeletion(ev);

		BPEvent *bp_ev;
		if((bp_ev = dynamic_cast<BPEvent*>(ev)) != NULL)
			m_Bp_cache.remove(bp_ev);
		m_BufferList.remove(ev);
		firelist_t::const_iterator it =
			std::find(m_FireList.begin(), m_FireList.end(), ev);
		if (it != m_FireList.end()) {
			m_DeleteList.push_back(ev);
		}
	}
}

EventList::iterator EventList::remove(iterator it)
{
	return (m_remove(it, false));
}

EventList::iterator EventList::m_remove(iterator it, bool skip_deletelist)
{
	if (!skip_deletelist) {
		// If skip_deletelist = true, m_remove was called from makeActive. Accordingly, we
		// are not going to delete an event, instead we are "moving" an event object (= *it)
		// from the buffer list to the fire-list. Therefor we only need to call the simulator's
		// event handler (m_onEventDeletion), if m_remove is called with the primary intention
		// to *delete* (not "move") an event.
		sal::simulator.onEventDeletion(*it);
		m_DeleteList.push_back(*it);
	}

	BPEvent *bp_ev;
	if((bp_ev = dynamic_cast<BPEvent*>(*it)) != NULL)
		m_Bp_cache.remove(bp_ev);
	return (m_BufferList.erase(it));
}

void EventList::remove(ExperimentFlow* flow)
{
	// WARNING: (*it) (= all elements in the lists) can be an invalid ptr because
	// clearEvents will be called automatically when the allocating experiment (i.e.
	// run()) has already ended. Accordingly, we cannot call
	//        sal::simulator.onEventDeletion(*it)
	// because a dynamic-cast of *it would cause a SEGFAULT. Therefor we require the
	// experiment flow to remove all residual events by calling clearEvents() (with-
	// in run()). As a consequence, we are now allowed to call the event-handler here.
	// See ExperimentFlow.hpp for more details.

	// all events?
	if (flow == 0) {
		for (bufferlist_t::iterator it = m_BufferList.begin();
		     it != m_BufferList.end(); it++)
			sal::simulator.onEventDeletion(*it); // invoke event handler
		m_Bp_cache.clear();
		m_BufferList.clear();
	} else { // remove all events corresponding to a specific experiment ("flow"):
		for (bufferlist_t::iterator it = m_BufferList.begin();
		     it != m_BufferList.end(); ) {
			if ((*it)->getParent() == flow) {
				sal::simulator.onEventDeletion(*it);
				it = m_BufferList.erase(it);
			} else {
				++it;
			}
		}
	}
	// events that just fired / are about to fire ...
	for (firelist_t::const_iterator it = m_FireList.begin();
		 it != m_FireList.end(); it++) {
		if (std::find(m_DeleteList.begin(), m_DeleteList.end(), *it)
		    != m_DeleteList.end()) {
			continue;  // (already in the delete-list? -> skip!)
		}
		// ... need to be pushed into m_DeleteList, as we're currently
		// iterating over m_FireList in fireActiveEvents() and cannot modify it
		if (flow == 0 || (*it)->getParent() == flow) {
			sal::simulator.onEventDeletion(*it);
			m_DeleteList.push_back(*it);
		}
	}
}

EventList::~EventList()
{
	// nothing to do here yet
}

BaseEvent* EventList::getEventFromId(EventId id)
{
	// Loop through all events:
	for(bufferlist_t::iterator it = m_BufferList.begin();
	    it != m_BufferList.end(); it++)
		if((*it)->getId() == id)
			return (*it);
	return (NULL); // Nothing found.
}

EventList::iterator EventList::makeActive(iterator it)
{
	assert(it != m_BufferList.end() &&
		   "FATAL ERROR: Iterator has already reached the end!");
	BaseEvent* ev = *it;
	assert(ev && "FATAL ERROR: Event object pointer cannot be NULL!");
	ev->decreaseCounter();
	if (ev->getCounter() > 0) {
		return ++it;
	}
	ev->resetCounter();
	// Note: This is the one and only situation in which remove() should NOT
	//       store the removed item in the delete-list.
	iterator it_next = m_remove(it, true); // remove event from buffer-list
	m_FireList.push_back(ev);
	return (it_next);
}

void EventList::fireActiveEvents()
{
	for (firelist_t::iterator it = m_FireList.begin();
		 it != m_FireList.end(); it++) {
		if (std::find(m_DeleteList.begin(), m_DeleteList.end(), *it)
		   == m_DeleteList.end()) { // not found in delete-list?
			m_pFired = *it;
			// Inform (call) the simulator's (internal) event handler that we are about
			// to trigger an event (*before* we actually toggle the experiment flow):
			sal::simulator.onEventTrigger(m_pFired);
			ExperimentFlow* pFlow = m_pFired->getParent();
			assert(pFlow && "FATAL ERROR: The event has no parent experiment (owner)!");
			sal::simulator.m_Flows.toggle(pFlow);
		}
	}
	m_FireList.clear();
	m_DeleteList.clear();
	// Note: Do NOT call any event handlers here!
}

size_t EventList::getContextCount() const
{
	set<ExperimentFlow*> uniqueFlows; // count unique ExperimentFlow-ptr
	for(bufferlist_t::const_iterator it = m_BufferList.begin();
		it != m_BufferList.end(); it++)
		uniqueFlows.insert((*it)->getParent());
	return (uniqueFlows.size());
}

} // end-of-namespace: fi
