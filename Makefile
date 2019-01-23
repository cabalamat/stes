# makefile for stes

# compile with RTTI off
FLAGS= -I$(INCLUDE)

OBJS = checkitem.o dataitem.o crypt.o setsi.o pstream.o
   
all: stesc stesd


#--------- executables:
        
stesc: $(OBJS) stesc.o 
	g++ -g stesc.o $(OBJS) -o stesc

stesd: $(OBJS) stesd.o 
	g++ -g stesd.o $(OBJS) -o stesd
        

#--------- ``main'' modules:
  
stesc.o: stesc.cpp checkitem.h dataitem.h stesdefs.h
	g++ $(FLAGS) -g -c stesc.cpp 
        
stesd.o: stesd.cpp checkitem.h stesdefs.h
	g++ $(FLAGS) -g -c stesd.cpp 

#--------- library modules:

crypt.o: crypt.cpp crypt.h 
	g++ $(FLAGS) -g -c crypt.cpp
        
checkitem.o: checkitem.cpp checkitem.h stesdefs.h pstream.h
	g++ $(FLAGS) -g -c checkitem.cpp
        
dataitem.o: dataitem.cpp dataitem.h pstream.h stesdefs.h
	g++ $(FLAGS) -g -c dataitem.cpp

setsi.o: setsi.cpp stesdefs.h 
	g++ $(FLAGS) -g -c setsi.cpp
        
pstream.o: pstream.cpp pstream.h 
	g++ $(FLAGS) -g -c pstream.cpp

#end
