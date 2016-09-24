/* */

parse arg infile outfile .

'@del' outfile

do while lines(infile) >0
   zeile = linein(infile)
   parse var zeile i_define i_symbol i_number .
   if i_define = '#define' & left(i_symbol, 6) = 'PANEL_' then
   do
      call lineout outfile, '.nameit symbol=' || left(i_symbol,30) 'text='|| i_number
   end
end

call stream infile, 'C', 'close'
call stream outfile, 'C', 'close'

return 0
