require 'mkmf'
dir_config('ical')
have_library('ical')
create_makefile('chaos_calendar')
