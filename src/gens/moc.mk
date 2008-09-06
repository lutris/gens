moc_%.cpp : %.h
	$(MOC) -i -o "$@" "$<"

clean-moc-extra:
	rm -vf moc_*.cpp

clean-am: clean-moc-extra
