SHELL = /bin/bash
include Makefile.variables

RM := rm -rf

all:
	@mkdir -p include
	@echo "MAKE lib"
	@cd lib && $(MAKE)
	@echo "MAKE MD src"
	@cd src && $(MAKE)
	@echo "MAKE MD examples"
	@cd examples && $(MAKE)
	@echo "MAKE tools"
	@cd tools && $(MAKE)

test: all
	# Do not run the database files
	$(RM) examples/examplebin/*database*
	@cd examples/examplebin; for i in ./*; do echo $$i && ./$$i >> $$i.log; done

clean:
	$(RM) *.log *.txt		# TODO: this will cause an issue if/when we change to cmake
	@echo "Cleaning lib"
	@cd lib && $(MAKE) clean
	@echo "Cleaning MD src and bin"
	@cd  src && $(MAKE) clean
	@echo "Cleaning MD Examples src/examples"
	@cd examples && $(MAKE) clean
	@echo "Cleaning bin"
	@cd bin && $(RM) *
	@echo "Cleaning include"
	@cd include && $(RM) *.o

debug:
	@echo "DEBUG BUILD"
	@echo "MAKE lib"
	@cd lib && $(MAKE)
	@echo "MAKE MD src"
	@cd src && $(MAKE) debug
	@echo "MAKE MD examples"
	@cd examples && $(MAKE)
	@echo "MAKE tools"
	@cd tools && $(MAKE)

clean_keep_data:
	@echo "Cleaning lib"
	@cd lib && $(MAKE) clean
	@echo "Cleaning MD src and bin"
	@cd  src && $(MAKE) clean
	@echo "Cleaning MD Examples src/examples"
	@cd examples && $(MAKE) clean_keep_data
