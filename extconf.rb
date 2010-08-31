require 'mkmf'
dir_config('ical')

if have_library("ical") or find_library("ical", nil, "/usr/local/include", "/opt/local/include" )
then
    create_makefile('chaos_calendar')
else
    puts "Error: libical could not be found!"
end
