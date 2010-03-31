SUFFIXES = .man .pod
.pod.man:
	pod2man --section=8 --release=$(PACKAGE_VERSION) --center "Experimentation Tools" $< $@

man8_MANS = krash.man

mostlyclean-local:
	-rm $(man8_MANS)

EXTRA_DIST = $(man8_MANS)


