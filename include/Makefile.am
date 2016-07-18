all-local:
	${SED} \
		-e "s|%CHAINLOADER%|${CHAINLOADER}|g" \
		-e "s|%PREFIX%|${prefix}|g" \
		-e "s|%LOCALSTATEDIR%|${localstatedir}|g" \
		-e "s|%BINDIR%|${bindir}|g" \
		-e "s|%SBINDIR%|${sbindir}|g" \
		-e "s|%LIBDIR%|${libdir}|g" \
		-e "s|%LIBEXECDIR%|${libexecdir}|g" \
		-e "s|%DATAROOTDIR%|${datarootdir}|g" \
		-e "s|%SYSCONFDIR%|${sysconfdir}|g" \
		-e "s|%PACKAGE%|${PACKAGE}|g" \
		-e "s|%PACKAGE_VERSION%|${PACKAGE_VERSION}|g" \
		config.h.in > config.h
