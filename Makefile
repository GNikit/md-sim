SHELL = /bin/bash


all:
	@echo "MAKE tools/TinyXML2"
	@cd tools/tinyxml2 && $(MAKE)
	@echo "MAKE MD src"
	@cd src && $(MAKE)
	@echo "MAKE MD examples"
	@cd examples && $(MAKE)

clean:
	@echo "Cleaning lib"
	@cd lib && $(MAKE) clean
	@echo "Cleaning MD src and bin"
	@cd  src && $(MAKE) clean
	@echo "Cleaning MD Examples src/examples"
	@cd examples && $(MAKE) clean

clean_keep_data:
	@echo "Cleaning lib"
	@cd lib && $(MAKE) clean
	@echo "Cleaning MD src and bin"
	@cd  src && $(MAKE) clean
	@echo "Cleaning MD Examples src/examples"
	@cd examples && $(MAKE) clean_keep_data
