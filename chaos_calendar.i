%module ChaosCalendar

%inline {
  VALUE occurrences( VALUE dtstart, VALUE dtend, char * rrule );
  VALUE duration_to_fixnum( char * duration );
}
