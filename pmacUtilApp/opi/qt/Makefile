install: button_go.png button_stop.png form_ui.py

button_p10.png: button.png
	convert $< -resize 78x100 -fill white -colorize 40% -font Helvetica -pointsize 18 -draw "gravity center fill grey text 15,-1 '+0.1' fill black text 16,0 '+0.1'" button_p01_up.png
	convert $< -resize 58x75 -fill white -colorize 65% -font Helvetica -pointsize 16 -draw "gravity center fill grey text 10,-1 '+1' fill black text 11,0 '+1'" button_p1_up.png
	convert $< -resize 39x50 -fill white -colorize 70% -font Helvetica -pointsize 10 -draw "gravity center fill grey text 7,-1 '+10' fill black text 8,0 '+10'" button_p10_up.png	
	convert $< -flop -resize 78x100 -fill white -colorize 40% -font Helvetica -pointsize 18 -draw "gravity center fill grey text -15,-1 '-0.1' fill black text -14,0 '-0.1'" button_m01_up.png
	convert $< -flop -resize 58x75 -fill white -colorize 65% -font Helvetica -pointsize 16 -draw "gravity center fill grey text -10,-1 '-1' fill black text -9,0 '-1'" button_m1_up.png
	convert $< -flop -resize 39x50 -fill white -colorize 70% -font Helvetica -pointsize 10 -draw "gravity center fill grey text -7,-1 '-10' fill black text -6,0 '-10'" button_m10_up.png	
	convert $< -resize 78x100 -fill white -colorize 10% -font Helvetica -pointsize 18 -draw "gravity center fill grey text 15,-1 '+0.1' fill black text 16,0 '+0.1'" button_p01.png
	convert $< -resize 58x75 -fill white -colorize 25% -font Helvetica -pointsize 16 -draw "gravity center fill grey text 10,-1 '+1' fill black text 11,0 '+1'" button_p1.png
	convert $< -resize 39x50 -fill white -colorize 40% -font Helvetica -pointsize 10 -draw "gravity center fill grey text 7,-1 '+10' fill black text 8,0 '+10'" button_p10.png	
	convert $< -flop -resize 78x100 -fill white -colorize 10% -font Helvetica -pointsize 18 -draw "gravity center fill grey text -15,-1 '-0.1' fill black text -14,0 '-0.1'" button_m01.png
	convert $< -flop -resize 58x75 -fill white -colorize 25% -font Helvetica -pointsize 16 -draw "gravity center fill grey text -10,-1 '-1' fill black text -9,0 '-1'" button_m1.png
	convert $< -flop -resize 39x50 -fill white -colorize 40% -font Helvetica -pointsize 10 -draw "gravity center fill grey text -7,-1 '-10' fill black text -6,0 '-10'" button_m10.png	
	convert $< -resize 78x100 -fill black -colorize 30% -font Helvetica -pointsize 18 -draw "gravity center fill grey text 15,-1 '+0.1' fill black text 16,0 '+0.1'" button_p01_down.png
	convert $< -resize 58x75 -fill black -colorize 20% -font Helvetica -pointsize 16 -draw "gravity center fill grey text 10,-1 '+1' fill black text 11,0 '+1'" button_p1_down.png
	convert $< -resize 39x50 -fill black -colorize 10% -font Helvetica -pointsize 10 -draw "gravity center fill grey text 7,-1 '+10' fill black text 8,0 '+10'" button_p10_down.png	
	convert $< -flop -resize 78x100 -fill black -colorize 30% -font Helvetica -pointsize 18 -draw "gravity center fill grey text -15,-1 '-0.1' fill black text -14,0 '-0.1'" button_m01_down.png
	convert $< -flop -resize 58x75 -fill black -colorize 20% -font Helvetica -pointsize 16 -draw "gravity center fill grey text -10,-1 '-1' fill black text -9,0 '-1'" button_m1_down.png
	convert $< -flop -resize 39x50 -fill black -colorize 10% -font Helvetica -pointsize 10 -draw "gravity center fill grey text -7,-1 '-10' fill black text -6,0 '-10'" button_m10_down.png	
		
button_go.png: Go.png	
	convert $< -font Helvetica -pointsize 30 -draw "gravity center fill white text 0,-1 'Go' fill grey text 1,0 'Go'" button_go.png
	convert $< -fill black -colorize 40% -font Helvetica -pointsize 30 -draw "gravity center fill white text 0,-1 'Go' fill grey text 1,0 'Go'" button_go_down.png
	
button_stop.png: Stop.png				
	convert $< -font Helvetica -pointsize 16 -draw "gravity center fill white text 0,-4 'Stop' fill grey text 1,-3 'Stop'" button_stop.png
	convert $< -fill black -colorize 50% -font Helvetica -pointsize 16 -draw "gravity center fill white text 0,-4 'Stop' fill grey text 1,-3 'Stop'" button_stop_down.png

button_reset.png: Stop.png				
	convert $< -font Helvetica -pointsize 16 -draw "gravity center fill white text 0,-4 'Reset' fill grey text 1,-3 'Reset'" button_reset.png
	convert $< -fill black -colorize 50% -font Helvetica -pointsize 16 -draw "gravity center fill white text 0,-4 'Reset' fill grey text 1,-3 'Reset'" button_reset_down.png

form_ui.py: form.ui
	pyuic4 $< > $@
						
clean:
	rm -f button_* form_ui.py *.pyc
