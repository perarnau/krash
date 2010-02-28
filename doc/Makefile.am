SUFFIXES = .man .pod
.pod.man:
	pod2man --section=8 --center "Experimentation Tools" $< $@

man8_MANS = krash.man

EXTRA_DIST = $(man8_MANS)


