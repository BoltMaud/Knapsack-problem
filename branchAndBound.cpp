#include <iostream>
#include <vector>
#include <algorithm>  
#include <string>
#include <fstream>
#include <sstream>

using namespace std;

/*
	this structure represente a objet with its number, weight, utility 
	UperW is the division of the utility per weight to do the branch and bound
*/
struct objectUtilityWeight{
	int number;
	float weight;
	float utility;
	float UperW;
};

/*
	this struct is the type of state of the algorithm branch and bound
	storedObjects : represente the objects that we accepted in the bag
	testObjects : represente the objects that we temporaily add in the bag
	testObject : is the object that we will try in the branch and bound tree
	weight : is the total weight of the state
	max : is the total utility
*/
struct result
{
    vector<objectUtilityWeight> storedObjects;
    vector<objectUtilityWeight> testObjects;
    objectUtilityWeight* testObject;
    float weight;
    float max;
};


/*
	this store the result
	its change during the algorithm
*/
result resultBag;


/*
	this function sort the objectUtilityWeight by utility per weight
*/
bool sortVector (objectUtilityWeight i,objectUtilityWeight j) { return (i.UperW<j.UperW); }


/*
	return the max result
*/
result max(result a,result b){
	if(a.max>b.max){
		return a;
	}
	else return b;
}

/*
	this function organize the objects by utility/weight
*/
vector<objectUtilityWeight> sortByValue(vector<float> & objectsWeight, vector<float> & objectsUtility ){
	vector<objectUtilityWeight> v;
	for(unsigned int i=0; i< objectsWeight.size(); i++){
		objectUtilityWeight obj;
		obj.weight=objectsWeight[i];
		obj.utility=objectsUtility[i];
		obj.UperW=objectsUtility[i]/objectsWeight[i];
		obj.number=i+1;
		v.push_back(obj);
	}
	sort (v.begin(), v.end(), sortVector);

	return v;
}

/*
	this function return true if a result can be a bag
	a bag can't have a half of object
*/
bool canBeABag(result r){
	return (bool)(r.testObject==nullptr);
}

/*
	this function prepare the future bag and include the tested object
*/
result withTestObject(result r,vector<objectUtilityWeight> * utilityWeightObjects){
	//keep the tested object
	r.storedObjects.push_back(*r.testObject);
	r.testObject=nullptr;
	unsigned int size=r.testObjects.size();
	for(unsigned int i=0; i<size;i++){
		utilityWeightObjects->push_back(r.testObjects.back());
		r.max-=r.testObjects.back().utility;
		//avoid problem with float
		if(r.max<0 || r.max<0.000001){
				r.max=0;
		}
		r.weight=r.weight-r.testObjects.back().weight;
			if(r.weight<0|| r.weight<0.000001){
				r.weight=0;
			}
		r.testObjects.pop_back();
	}
    return r;
}

/*
	this function prepare the future bag but doesn't include the tested object
*/
result withoutTestObject(result r,vector<objectUtilityWeight> * utilityWeightObjects){
    //delete object
	r.max-=r.testObject->utility;
	r.weight-=r.testObject->weight;
	r.testObject=nullptr;

	if(r.testObjects.size()!=0){
		unsigned int size=r.testObjects.size();
		for(unsigned int i=0; i<size;i++){
			r.max=r.max-r.testObjects.back().utility;
			//avoid problem with float
			if(r.max<0 || r.max<0.000001){
				r.max=0;
			}
			r.weight=r.weight-r.testObjects.back().weight;
			if(r.weight<0 || r.weight<0.000001){
				r.weight=0;
			}
			utilityWeightObjects->push_back(r.testObjects.back());
			r.testObjects.pop_back();
		}
	}
	return r;	
}

/*
	this function return a copy of the type result
	this is usefull to do the two branchs of the algorithm
*/
result copyResult(result newBag){
		result newBag2;
		newBag2.max=newBag.max;
		newBag2.weight=newBag.weight;
		newBag2.storedObjects=newBag.storedObjects;
		newBag2.testObjects=newBag.testObjects;
		objectUtilityWeight *testObject=new objectUtilityWeight() ;
		testObject->number=newBag.testObject->number;
		testObject->UperW=newBag.testObject->UperW;
		testObject->weight=newBag.testObject->weight;
		testObject->utility=newBag.testObject->utility;
		newBag2.testObject=testObject;
		return newBag2;
}

/*
	this is the main function 
	its impletements the branch and bound algorithm for a bag
*/
void maxUtilityMinWeight(result structBag,vector<objectUtilityWeight> & utilityWeightObjects, float capacity){
	//prepare a new bag
	result newBag;

	//init the new bag
	newBag.weight=structBag.weight;
	newBag.max=structBag.max;
	newBag.testObject=nullptr;
	newBag.storedObjects=structBag.storedObjects;
	
	//while we can add a object
	while(newBag.weight<capacity && (utilityWeightObjects.size()!=0)){
		newBag.testObjects.push_back(utilityWeightObjects.back());
		newBag.weight+=utilityWeightObjects.back().weight;
		newBag.max+=utilityWeightObjects.back().utility;
		utilityWeightObjects.pop_back();
	}

	//if the bag is to heavy
	if(newBag.weight>capacity){
		//put the last object in the tested object
		newBag.testObject= &(newBag.testObjects.back()) ; 
		newBag.testObjects.pop_back();
	}
	
	//if the bag has only completed objets
	if(canBeABag(newBag)){
		//compare to the stored bag
		if(newBag.max>resultBag.max){
			resultBag=newBag;
			resultBag.storedObjects=newBag.testObjects;
			resultBag.storedObjects.insert(resultBag.storedObjects.end(),newBag.storedObjects.begin(),newBag.storedObjects.begin());

		}
	}
	//if the bag has uncompleted objects
	else{
		//if there is only an tested object (the one in testObject value)
		if(newBag.testObjects.size()==0){
			//if the bag is to heavy
			if(newBag.weight >capacity){
				//the object is rejected
				newBag=withoutTestObject(newBag,&utilityWeightObjects);
				maxUtilityMinWeight( newBag,utilityWeightObjects,capacity);
			}
		}
		//if there are objects to test
		else{
			//do a copy of the newbag
			result newBag2=copyResult(newBag);
			vector<objectUtilityWeight>  utilityWeightObjects2=utilityWeightObjects;
			
			//try to add the object to the bag
			if(newBag.weight+newBag.testObject->weight<=capacity){
				newBag=withTestObject( newBag,&utilityWeightObjects);
				maxUtilityMinWeight( newBag,utilityWeightObjects,capacity);
			}

			//try without the object	
			newBag2=withoutTestObject(newBag2,&utilityWeightObjects2);
			maxUtilityMinWeight( newBag2,utilityWeightObjects2,capacity);
		}
	}		
}


int main(int argc, char* argv[])
{
	
	if(argc>1){
		//find the file
		std::ifstream source;
		source.open( argv[1]);
		std::string line;

		//declare variables 
		vector<float> objectsUtility ={};
		vector<float> objectsWeight = {};
		float capacity;


		//read the test 
		while (std::getline(source, line)){
			//note the utilities
			if(line[0]=='u'){
				std::stringstream iss( line.substr(4,line.size()-1) );
				while ( iss.good() ){
					string number;
   					getline( iss, number, ',' );
  					objectsUtility.push_back( std::stof(number) );
				}
				
			}
			//note the weights
			if(line[0]=='w'){
				std::stringstream iss( line.substr(4,line.size()-1) );
				while ( iss.good() ){
					string number;
   					getline( iss, number, ',' );
  					objectsWeight.push_back( std::stof(number) );
				}

			}
			//note the capacity
			if(line[0]=='W'){
				capacity=std::stof(line.substr(2,line.size()));
			}
		}

		//make the test 
		//sort the object by utility/weight
		vector<objectUtilityWeight> v=sortByValue(objectsWeight,objectsUtility);

		//init the result
		resultBag.max=0;
   		resultBag.weight=0;
   		resultBag.testObject=nullptr;

		//do the branch and bound algorithm
		maxUtilityMinWeight(resultBag,v,capacity);
    	
		//display the result
		cout <<"sol="<< resultBag.max<< " -> objects : ";
		for(unsigned int i = 0; i < resultBag.storedObjects.size(); i++){
      		  cout << resultBag.storedObjects[i].number<< " " ;
    	}
		cout << " à choisir "<<endl;

	}
	else{
		cout << "please see the README.txt"<< endl;
	}

}
