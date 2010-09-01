#include <ruby.h>
#include <ical.h>
#include <time.h>

//#define RRULE   "FREQ=MONTHLY;BYMONTH=1,2,3,4,5,6,7,8,9,10,11;BYDAY=-1WE;UNTIL=20091105T220000"
//#define RRULE   "FREQ=DAILY;UNTIL=20991111T220000;VFOO"

/* Ruby house keeping */
VALUE occurrences( VALUE self, VALUE dtstart, VALUE dtend, VALUE rrule );
VALUE duration_to_fixnum( VALUE self, VALUE duration );
static VALUE mChaosCalendar;
void Init_chaos_calendar() {
  mChaosCalendar = rb_define_module("ChaosCalendar");

  rb_define_module_function(mChaosCalendar, "occurrences", occurrences, 3);
  rb_define_module_function(mChaosCalendar, "duration_to_fixnum", duration_to_fixnum, 1);
}

static VALUE to_time( VALUE input, char * name ) {
  ID time_to_time    = rb_intern( "to_time" );
  ID time_tv_sec     = rb_intern( "tv_sec" );
  ID time_parse;
  VALUE time_class;

  if( rb_respond_to( input, time_tv_sec ) ) return input;
  if( rb_respond_to( input, time_to_time ) ) return rb_funcall( input, time_to_time, 0 );

//  if( rb_require( "Time" ) != Qtrue )
//    rb_raise( rb_eRuntimeError, "Can't find class Time" );
  rb_require( "Time" );
  time_parse = rb_intern( "parse" );

  if( rb_respond_to( rb_cTime, time_parse ) )
    return rb_funcall( rb_cTime, time_parse, 1, input );

  rb_raise( rb_eTypeError, "Can't convert %s into a Time-like object.", name );
  /* Never reached */
  return Qnil;
}

VALUE occurrences( VALUE self, VALUE dtstart, VALUE dtend, VALUE rrule ) {
  char * _rrule;
  struct icaltimetype start, end;
  time_t tt;
  VALUE  tv_sec, occurr = rb_ary_new();

  /* Get method ID for Time.tv_sec */
  ID time_tv_sec  = rb_intern( "tv_sec" );
  ID to_string    = rb_intern( "to_string" );

  if( TYPE( rrule ) != T_STRING && rb_respond_to( rrule, to_string ) )
    rrule = rb_funcall( rrule, to_string, 0 );

  Check_Type(rrule, T_STRING);
  _rrule = RSTRING(rrule)->ptr;

  dtstart = to_time( dtstart, "dtstart" );
  dtend   = to_time( dtend,   "dtend" );

  /* Apply .tv_sec to our Time objects (if they are Times ...) */
  tv_sec = rb_funcall( dtstart, time_tv_sec, 0 );
  tt     = NUM2INT( tv_sec );
  start  = icaltime_from_timet( tt, 0 );

  tv_sec = rb_funcall( dtend, time_tv_sec, 0 );
  tt     = NUM2INT( tv_sec );
  end    = icaltime_from_timet( tt, 0 );

  icalerror_clear_errno();
  icalerror_set_error_state( ICAL_MALFORMEDDATA_ERROR, ICAL_ERROR_NONFATAL);

  struct icalrecurrencetype recur = icalrecurrencetype_from_string( _rrule );
  if( icalerrno != ICAL_NO_ERROR ) {
    rb_raise(rb_eArgError, "Malformed RRule");
    return Qnil;
  }

  icalrecur_iterator* ritr = icalrecur_iterator_new( recur, start );

  while(1) {
    struct icaltimetype next = icalrecur_iterator_next(ritr);

    if( icaltime_is_null_time(next) || ( icaltime_compare( next, end ) > 0 ) ) {
      icalrecur_iterator_free(ritr);
      return occurr;
    }

    rb_ary_push( occurr, rb_time_new( icaltime_as_timet( next ), 0 ) );
  };

  icalrecur_iterator_free(ritr);
  return occurr;
}

VALUE duration_to_fixnum( VALUE self, VALUE duration ) {
  struct icaldurationtype dur_struct;
  char * _duration;

  ID to_string    = rb_intern( "to_string" );

  if( TYPE( duration ) != T_STRING && rb_respond_to( duration, to_string ) )
    duration = rb_funcall( duration, to_string, 0 );

  Check_Type(duration, T_STRING);
  _duration = RSTRING(duration)->ptr;

  icalerror_clear_errno();
  icalerror_set_error_state( ICAL_MALFORMEDDATA_ERROR, ICAL_ERROR_NONFATAL);

  dur_struct = icaldurationtype_from_string( _duration );
  if( icaldurationtype_is_bad_duration( dur_struct ) )
    rb_raise(rb_eArgError, "Malformed Duration");

  return LONG2FIX(icaldurationtype_as_int( dur_struct ));
}
