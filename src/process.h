#ifndef PROCESS_H
#define PROCESS_H

class Process {
protected:
	int rank, size;
public:
	virtual int run(int, int);
};

class Alcoholic: public Process {
private:
	Alcoholic();
	~Alcoholic();
public:
	static Alcoholic & getInstance();
	int run(int, int);
};

class SocialWorker: public Process {
private:
	SocialWorker();
	~SocialWorker();
public:
	static SocialWorker & getInstance();
	int run(int, int);
};

#endif