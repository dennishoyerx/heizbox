#include "Observable.h"

// Explizite Template-Instanziierung für häufig genutzte Typen
// Dies kompiliert die Templates NUR EINMAL statt bei jeder Nutzung

// Observable
template class Observable<bool>;
template class Observable<uint8_t>;
template class Observable<uint16_t>;
template class Observable<int8_t>;
template class Observable<int16_t>;
template class Observable<uint32_t>;
template class Observable<int32_t>;
template class Observable<float>;
template class Observable<double>;

// PersistedObservable
template class PersistedObservable<bool>;
template class PersistedObservable<uint8_t>;
template class PersistedObservable<uint16_t>;
template class PersistedObservable<int8_t>;
template class PersistedObservable<int16_t>;
template class PersistedObservable<uint32_t>;
template class PersistedObservable<int32_t>;
template class PersistedObservable<float>;
template class PersistedObservable<double>;