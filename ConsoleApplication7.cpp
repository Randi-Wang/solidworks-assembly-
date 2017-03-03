#include "stdafx.h"
#include "atlstr.h"
#include "atlsafe.h"
//#include <variant>
#include <string>
#include <iostream>
#include <vector>
#include <fstream>


#include <wtypes.h>
#include <comutil.h>
#pragma comment(lib,"comsuppw.lib")

#include <string.h>
#include <stdio.h>


VARIANT_BOOL retVal = VARIANT_FALSE;
HRESULT hres = NOERROR;

boolean metalIsTrue=false;
boolean smallVolisTrue=false;
vector<string> materialNameList;




void OpenAssembly(ISldWorks* swApp, IModelDoc2** swModel);
//void TraverseModelFeatures(IModelDoc2** swModel, long nLevel);


void TraverseComponents(IComponent2 *recComp, int level, ISldWorks* swApp);
void TraverseComponentFeatures(IComponent2* swComp, long nLevel);
void TraverseFeatureFeatures(IFeature* swFeat, long nLevel);
void GetBoundingBox(IComponent2 *m_childComponent);
void GetMate(IComponent2 *component);
void GetPropertyofPart(IComponent2 *m_childComponent);
void SuppressNonMetalComponent(IComponent2* swComponent, ISldWorks* swApp);
void GetMaterialDB(ISldWorks* swApp);
//boolean JudgementofMetal();
char* ConvertBSTRToLPSTR(BSTR bstrIn);
string bstr_to_str(BSTR source);

//void TraverseMateFeatures(IFeature* swFeat, long nLevel);



using namespace std;


//int main()
//{
//    return 0;
//}

int _tmain(int argc, _TCHAR* argv[])

{




	// To initialize COM

	// Do this before using ATL smart pointers so COM is available.

	CoInitialize(NULL);
	
	// Use a block, so the smart pointers are destructed when the scope of this block is left

	{

		//Use ATL smart pointers
		CComPtr<ISldWorks> swApp;
		CComPtr<IModelDoc2> swModel; //swModel is a pointer to ModelDoc.
		CComPtr<IComponent2> swComponent;
		CComPtr<IAssemblyDoc> swAssemblyDoc;

		CComPtr<IConfigurationManager> swConfMgr;
		CComPtr<IConfiguration> swConf;
		CComPtr<IComponent2> swRootComp;
		
		VARIANT_BOOL swRootCompBool=true;
		

		hres = swApp.CoCreateInstance(__uuidof(SldWorks), NULL, CLSCTX_LOCAL_SERVER);

		if (hres != S_OK)
			throw 0;


		//Open assembly function
		OpenAssembly(swApp, &swModel);

		
		swApp->put_UserControl(VARIANT_TRUE);

		swApp->put_Visible(VARIANT_TRUE);

		if (!swModel) {

			return(0);

		}

		/*vector<string> materialNameList;*/

		ifstream ifs("C:\\Users\\rwang\\Google Drive\\PARC\\Research\\Defeature\\NonMetalMaterials.txt");
		string currLine;

		while (getline(ifs, currLine)) {
			materialNameList.push_back(currLine);

		}


		////output the materialNameList

		//for (vector<string>::const_iterator i = materialNameList.begin(); i != materialNameList.end(); ++i) {
		//	cout << *i << endl;
		//}
	
		



		



		CComBSTR  strModelTitle;
		long      nDocumentType;  // swDocumentTypes_e

		swModel->GetTitle(&strModelTitle); // get title of the assembly model
		swModel->GetType(&nDocumentType); // get type of the document

		CComPtr<IFeature>  swFeature;
		CComPtr<IFeature>  swSubFeature;

		CComBSTR           strFeatureName;
		CComBSTR           strFeatureType;
		CComBSTR           strMaterialName;

		//get first feature of solidworks model

		swModel->IFirstFeature(&swFeature);

		CComPtr<IFeature>  swFeatureManager;

		CComPtr<IFeature>  swSubFeatureManager;

		CComBSTR           strFeatureManagerName;

		// iteration of feature tree of Solidworks Model.

		TraverseFeatureFeatures(swFeature, 1);

		swModel->get_ConfigurationManager(&swConfMgr);
		swConfMgr->get_ActiveConfiguration(&swConf);
		swConf->GetRootComponent3(swRootCompBool, &swRootComp); //get root component
	
		//iteration of feature tree of Solidworks Model.
		TraverseComponents(swRootComp, 1, swApp);

		printf("please select AT MOST one part(assembly) in solidworks, then press ENTER \n");

		//system pauses, give user time to selcte part(subassembly)
		system("pause");


		//remember the selected part(subassembly) and output its bounding box

		CComPtr<ISelectionMgr>     swSelectionManager;

		long                       lNumSelections;

		long                       nSelectionType;     // swSelectType_e

		//CComPtr<IDispatch>         swSelectedObject;
		CComPtr<IFace2>            selectedFace;
		CComPtr<IEdge>             selectedEdge;
		/*CComPtr<IComponent2>	   selectedComponents;*/
		CComPtr<IComponent2>	   currentComponents;
		long componentNum = 0;

		swModel->get_ISelectionManager(&swSelectionManager);
		swSelectionManager->GetSelectedObjectCount2(-1, &lNumSelections);

		//remember the selected part(subassembly). 
		//User can also choose faces, edges, but there is no action for them.
		for (int i = 1; i <= lNumSelections; i++) {
			CComPtr<IDispatch>         swSelectedObject;
			CComPtr<IComponent2>	   selectedComponents;
			swSelectionManager->GetSelectedObject6(i, -1, &swSelectedObject);
			swSelectionManager->GetSelectedObjectType3(i, -1, &nSelectionType);

			switch (nSelectionType) {
			case swSelectType_e::swSelCOMPONENTS:

				//hres = swSelectedObject->QueryInterface(__uuidof(IComponent2), reinterpret_cast<void**>(&selectedComponents));

				swSelectedObject.QueryInterface(&selectedComponents);
				GetBoundingBox(selectedComponents);
				GetMate(selectedComponents);
				GetPropertyofPart(selectedComponents);
				componentNum++;
				break;

			case swSelectType_e::swSelFACES:

				swSelectedObject.QueryInterface(&selectedFace);

				break;

			case swSelectType_e::swSelEDGES:

				swSelectedObject.QueryInterface(&selectedEdge);

				break;

			}

		}

	}

	// ATL smart pointers are destructed so all COM objects you held on to are released

	// Now you can safely shutdown COM as you do not need it any longer

	// Stop COM

	CoUninitialize();


	return(0);

}


void OpenAssembly(ISldWorks* swApp, IModelDoc2** swModel)
//Open assembly 
{
	CComBSTR sFileName(L"C:\\Users\\rwang\\Google Drive\\PARC\\Research\\CoAnalysisArchive\\Solidworks\\solidworks\\TEST\\Assem2.SLDASM");
	


	CComBSTR sDefaultConfiguration(L"Default");



	long fileerror, filewarning;

	IModelDoc2* swModelAssembly;
	hres = swApp->OpenDoc6(sFileName, swDocASSEMBLY, swOpenDocOptions_Silent, sDefaultConfiguration, &fileerror, &filewarning, &swModelAssembly);
	*swModel = swModelAssembly; //Set the value of the input argument equal to address of the interface
	//GetMaterialDB(swApp);
	
}

//void GetMaterialDB(ISldWorks* swApp) {
//	
//	string *data = NULL;
//	VARIANT database;
//	hres = swApp->GetMaterialDatabases(&database);
//	SAFEARRAY* datapsa = V_ARRAY(&database);
//	HRESULT datares = SafeArrayAccessData(datapsa, (void **)&data);
//	
//
//	//BSTR path;
//	//hres = swApp->GetMaterialSchemaPathName(&path);
//	//printf("%S", path);
//	
//	//printf("%S", data[2]);
//	//printf("%S", data[3]);
//	//printf("%S", data[4]);
//	//printf("%S", data[5]);
//	//printf("%S", data[6]);
//	//printf("%S", data[7]);
//	//for (int i = 0; i < (*data).size(); i++) {
//	//	printf("S%", data[i]);
//	//}
//
//}

void TraverseComponents(IComponent2 *recComp, int level, ISldWorks* swApp)
{// input the root component. get all the components of the tree.

	typedef IComponent2 *LPCOMPONENT2;
	CComBSTR name, cfg, material;
	int count = 0;

	//get the number of children of a component(subassembly)

	recComp->IGetChildrenCount(&count);

	if (!count)
		return;

	LPDISPATCH* componentChildrenArray;
	VARIANT vChildren;
	HRESULT hres = recComp->GetChildren(&vChildren);
	SAFEARRAY* psa = V_ARRAY(&vChildren);
	HRESULT res = SafeArrayAccessData(psa, (void **)&componentChildrenArray);
	LPCOMPONENT2 m_childComponent = NULL;


	//LPDISPATCH* componentMatesArray;
	//VARIANT vMates;
	//HRESULT hresMates = recComp->GetMates(&vMates);
	//SAFEARRAY* psaMates = V_ARRAY(&vMates);
	//HRESULT resMates = SafeArrayAccessData(psaMates, (void **)&componentMatesArray);
	//LPCOMPONENT2 m_componentMates = NULL;

	for (int i = 0; i < count; i++)
	{
		componentChildrenArray[i]->QueryInterface(__uuidof(IComponent2), (void**)&m_childComponent);

		if (m_childComponent != NULL)
		{
			componentChildrenArray[i]->AddRef();

			m_childComponent->get_Name2(&name);
			//m_childComponent->GetMates(&vMates);
			//m_childComponent->GetMaterialIdName(&material);
			//printf("material Name: %S \n", material);

			//m_childComponent->get_ReferencedConfiguration(&cfg);
			printf("Part(subassembly) Name: %S \n", name);

			TraverseComponentFeatures(m_childComponent, level);

			GetBoundingBox(m_childComponent);
			printf(" \n");

			TraverseComponents(m_childComponent, level + 1, swApp);
			if (metalIsTrue == true) {
				SuppressNonMetalComponent(m_childComponent, swApp);
			}
			GetPropertyofPart(m_childComponent);
			if (smallVolisTrue == true) {
				SuppressNonMetalComponent(m_childComponent, swApp);
			}
		}

		m_childComponent = NULL;
	}
}


void TraverseComponentFeatures(IComponent2* swComp, long nLevel) {
// input a component, get its first feature for traversing all its features shown in the tree.

	CComPtr<IFeature> swfeat;

	hres = swComp->FirstFeature(&swfeat);

	//traverse all its features, subfeatures, subfeatures...
	TraverseFeatureFeatures(swfeat, nLevel);


	//TraverseMateFeatures(swfeat, nLevel);

}

void TraverseFeatureFeatures(IFeature* swFeat, long nLevel) {
//Going from first feature, output all other features of a component.

	/*HRESULT hres;*/
	/*string sPadStr;*/
	//long i;

	CComPtr<IFeature>  strFeature;

	strFeature = swFeat;

	CComPtr<IFeature>  swSubFeature;

	CComBSTR  strFeatureName;	
	CComBSTR  strFeatureType;

	CComBSTR  strSubFeatureName;
	CComBSTR  strSubFeatureType;

	IFeature* featuretype;
	IFeature* featureName;

	IFeature* swMateFeature;

	string tempName;

	//IDispatch *pDisp;
	//CComBSTR  strMateFeatureName;
	//CComBSTR  strMateFeatureType;
	//IMate2* swMate;
	//

	//LPDISPATCH* componentChildrenArray;
	//VARIANT vChildren;
	//HRESULT hres = recComp->GetChildren(&vChildren);
	//SAFEARRAY* psa = V_ARRAY(&vChildren);
	//HRESULT res = SafeArrayAccessData(psa, (void **)&componentChildrenArray);
	//LPCOMPONENT2 m_childComponent = NULL;

	//for (i = 0; i < nLevel; ++i) {
	//	sPadStr = sPadStr + string(" ");
	//}
	while (strFeature) {

		strFeature->GetTypeName2(&strFeatureType);
		if (strFeatureType == "MaterialFolder") {
			strFeature->get_Name(&strFeatureName);
			printf("Material Property: %S\n", strFeatureName);
		}
		
		if (strFeatureName != NULL) {
			tempName = bstr_to_str(strFeatureName);
			//cout << tempName << endl;



			//char* covertedName;
			//covertedName = ConvertBSTRToLPSTR(strFeatureName);
			//printf("%c", covertedName);



			if (find(materialNameList.begin(), materialNameList.end(), tempName) != materialNameList.end())
			{
				metalIsTrue = true;
				break;
			}
			else {
				metalIsTrue = false;
			}
		}
		
		

		/*for (vector<string>::const_iterator i = materialNameList.begin(); i != materialNameList.end(); ++i) {


			if (strFeatureName == "*i") {
				cout << *i << endl;
				metalIsTrue = true;
				break;
			}
			else {
				metalIsTrue = false;
			}

		}*/

		

		/*		if (strFeatureType == "MateGroup") {
		swMateFeature = strFeature;
		}
		*/

		strFeature->IGetFirstSubFeature(&swSubFeature);


		while (swSubFeature) {

			swSubFeature->GetTypeName2(&strSubFeatureType);
			if (strSubFeatureType == "MaterialFolder")
			{
				swSubFeature->get_Name(&strSubFeatureName);
				//printf("Material Property: %S\n", (wchar_t*)strSubFeatureName);
			}

			/*for (vector<string>::const_iterator i = materialNameList.begin(); i != materialNameList.end(); ++i) {

				
				if (strFeatureName == "*i" || strSubFeatureName == "*i") {
					metalIsTrue = true;
					break;
					
				}
				else {
					metalIsTrue = false;
				}

			}*/
			//	cout << *i << endl;
			//}

			
			//if (strSubFeatureType == "MateGroup") {
			//	strFeature->GetSpecificFeature2(&pDisp);
			//	hres = pDisp->QueryInterface(IID_IFeature, (void**)&swSubFeature);
			//	pDisp = NULL;
			//}


			CComPtr<IFeature>  swNextSubFeature;
			swSubFeature->IGetNextSubFeature(&swNextSubFeature);
			swSubFeature = swNextSubFeature;//close loop for subfeature
		}

		CComPtr<IFeature>  swNextFeature;
		strFeature->IGetNextFeature(&swNextFeature);
		strFeature = swNextFeature;//close loop for feature


	}
	
}

//boolean JudgementofMetal() {
//	metalIsTrue=true;
//	return metalIsTrue;
//}


void SuppressNonMetalComponent(IComponent2* swComponent, ISldWorks* swApp)

//Suppress selected component if it is non-matal

{

	CComBSTR messageSuppressed(L"Component is already suppressed.");



	long lComponentSuppressState;

	long lSuppressMessageResult;

	long lSuppressError;



	hres = swComponent->GetSuppression(&lComponentSuppressState);

	if (lComponentSuppressState == swComponentSuppressed) {


		hres = swApp->SendMsgToUser2(messageSuppressed, swMbInformation, swMbOk, &lSuppressMessageResult);
		//metalIsTrue = false;
	}
	else

		hres = swComponent->SetSuppression2(swComponentSuppressed, &lSuppressError);

}

void GetBoundingBox(IComponent2 *m_childComponent) {
// input a component, get its bounding box.
	if (m_childComponent != NULL) {

		double *pData = NULL;
		LPDISPATCH* BoxArray;
		VARIANT box;
		HRESULT hresBox = m_childComponent->GetBox(false, false, &box);
		SAFEARRAY* psaBox = V_ARRAY(&box);
		HRESULT resBox = SafeArrayAccessData(psaBox, (void **)&pData);

		
		long minIndex = 0;
		long maxIndex = 0;
		int rei = 0;
		CComBSTR CompName;

		/*hresBox = SafeArrayGetUBound(psaBox, 1, &minIndex);
		hresBox=SafeArrayGetLBound(psaBox,  1, &minIndex);*/

		hres = m_childComponent->get_Name2(&CompName);
		

		if (pData == NULL) 
			return;
		else {
			
			printf("%S 's Bounding Box: X_Corner1, Y_Corner1, Z_Corner1, X_Corner2, Y_Corner2, Z_Corner2  =", CompName);

			for (int i = 0; i < 6; i++) {

				printf("  %lf", pData[i]);
			}

			printf("\n");
		
		
		}
		

	}

}


void GetPropertyofPart(IComponent2 *m_childComponent) {
	// input a component, get its bounding box.
	if (m_childComponent != NULL) {

		IDispatch* modelDocDispatch;
		hres = m_childComponent->GetModelDoc2(&modelDocDispatch);
		IModelDoc2Ptr modelDoc = NULL;
		long wheretaken=0;
		double volume;
		double density;
		double mass;

		double *dataCenterOfMass = NULL;
		//double *dataIGetPrincipleMomentsOfInertia = NULL;
		double *dataPrincipleInertial_X = NULL;
		double *dataPrincipleInertial_Y = NULL;
		double *dataMomentofInertia = NULL;

		VARIANT CenterOfMass;
		VARIANT PrincipleInertial_X;
		VARIANT PrincipleInertial_Y;
		VARIANT MomentofInertia;

		if (modelDocDispatch != NULL) {
			hres = modelDocDispatch->QueryInterface(IID_IModelDoc2, (void**)&modelDoc);

			IModelDocExtensionPtr modelDocExt = NULL;
			hres = modelDoc->get_Extension(&modelDocExt);

			IMassPropertyPtr massProperty = NULL;
			modelDocExt->CreateMassProperty(&massProperty);

			massProperty->get_Volume(&volume);
			massProperty->get_Mass(&mass);
			massProperty->get_Density(&density);
			massProperty->get_CenterOfMass(&CenterOfMass);
			SAFEARRAY* psaCenterOfMass = V_ARRAY(&CenterOfMass);
			HRESULT resCenterOfMass = SafeArrayAccessData(psaCenterOfMass, (void **)&dataCenterOfMass);
			
	/*		massProperty->IGetPrincipleMomentsOfInertia(dataIGetPrincipleMomentsOfInertia);
			printf("%lf", dataIGetPrincipleMomentsOfInertia[0]);
			*/
			//for (int i = 0; i < 3; i++) {

			//	printf("%lf", dataIGetPrincipleMomentsOfInertia[i]);
			//}
			
			massProperty->get_PrincipleAxesOfInertia(0, &PrincipleInertial_X);
			SAFEARRAY* psaPrincipleInertial_X = V_ARRAY(&PrincipleInertial_X);
			HRESULT resPrincipleInertial_X = SafeArrayAccessData(psaPrincipleInertial_X, (void **)&dataPrincipleInertial_X);

			massProperty->get_PrincipleAxesOfInertia(1, &PrincipleInertial_Y);
			SAFEARRAY* psaPrincipleInertial_Y = V_ARRAY(&PrincipleInertial_Y);
			HRESULT resPrincipleInertial_Y = SafeArrayAccessData(psaPrincipleInertial_Y, (void **)&dataPrincipleInertial_Y);


			massProperty->GetMomentOfInertia(0,&MomentofInertia);
			SAFEARRAY* psaMomentofInertia = V_ARRAY(&MomentofInertia);
			HRESULT resMomentofInertia = SafeArrayAccessData(psaMomentofInertia, (void **)&dataMomentofInertia);

			if (volume <= 0.000035) {
				smallVolisTrue = true;
			}
			else {
				smallVolisTrue = false;
			}
			/*printf("volume of this part: %lf \n ", volume);
			printf("density of this part: %lf \n ", density);*/


			/*printf("CenterOfMass:");
			for (int i = 0; i < 3; i++) {

				printf("%lf", dataCenterOfMass[i]*100000);
			}

			printf("\n");


			printf("PrincipleAxesOfInertia:");
			for (int i = 0; i < 3; i++) {

				printf("%lf", dataMomentofInertia[i]*10000000000);
			}
*/
			//printf("\n");

			//printf("PrincipleAxesOfInertia: %lf \n", dataPrincipleInertial_X[0]+ dataPrincipleInertial_X[1]+ dataPrincipleInertial_X[2]);
			//printf("PrincipleAxesOfInertia: %lf \n", dataPrincipleInertial_Y[0] + dataPrincipleInertial_Y[1] + dataPrincipleInertial_Y[2]);

			//printf("\n");


			//printf("MomentofInertia:");
			/*f*//*or (int i = 0; i < 9; i++) {

				printf("%lf", dataMomentofInertia[i] * 1000000000000);
				printf(",");
				if (i == 2 || i == 5) {
					printf("\n");
				}
			}
			printf("\n");*/

		}
		else {
			// error
			volume = 0;
			density = 0;
			mass = 0;
		}


	}

}



void GetMate(IComponent2 *component) {
	VARIANT compMates2;
	SAFEARRAY* pSafeArrayOfMates2 = NULL;
	CComSafeArray<IDispatch*> mMate;
	//IDispatch* mate2;
	//CComPtr<IDispatch*> mate2;
	SAFEARRAYBOUND* psafearraybound;
	CComPtr<IMate2> mate2;

	//LPDISPATCH* MateArray;
	//HRESULT res = SafeArrayAccessData(mMate, (void **)&MateArray);
	//Mate2* mates = NULL;

	hres=component->GetMates(&compMates2);
	pSafeArrayOfMates2 = V_ARRAY(&compMates2);
	mMate = V_ARRAY(&compMates2);
	long nn;
	long mType;
	nn = sizeof(&mMate);
	psafearraybound= &((pSafeArrayOfMates2->rgsabound)[0]);  ///these 2 lines give the size of the mate array
	unsigned long numOfMates = psafearraybound->cElements;

	for (long i = 0; i < numOfMates; ++i) { ///this for loop iterates and gets the type of mate

		
		mate2 = mMate[i];
		
		mate2->get_Type(&mType); //this works
	}
	
}


char* ConvertBSTRToLPSTR(BSTR bstrIn)
{
	LPSTR pszOut = NULL;

	if (bstrIn != NULL)
	{
		int nInputStrLen = SysStringLen(bstrIn);

		// Double NULL Termination
		int nOutputStrLen = WideCharToMultiByte(CP_ACP, 0, bstrIn, nInputStrLen, NULL, 0, 0, 0) + 2;

		pszOut = new char[nOutputStrLen];

		if (pszOut)
		{
			memset(pszOut, 0x00, sizeof(char)*nOutputStrLen);

			WideCharToMultiByte(CP_ACP, 0, bstrIn, nInputStrLen, pszOut, nOutputStrLen, 0, 0);
		}
	}

	return pszOut;
}

string bstr_to_str(BSTR source) {
	//source = L"lol2inside";
	_bstr_t wrapped_bstr = _bstr_t(source);
	int length = wrapped_bstr.length();
	char* char_array = new char[length];
	strcpy_s(char_array, length + 1, wrapped_bstr);
	return char_array;
}

// Backup code for searching mates

//LPDISPATCH* componentMatesArray;
//VARIANT vMates;
//HRESULT hresMates = recComp->GetMates(&vMates);
//SAFEARRAY* psaMates = V_ARRAY(&vMates);
//HRESULT resMates = SafeArrayAccessData(psaMates, (void **)&componentMatesArray);
//LPCOMPONENT2 m_componentMates = NULL;

//for (int i = 0; i < count; i++)
//{
//	componentMatesArray[i]->QueryInterface(__uuidof(IComponent2), (void**)&m_componentMates);

//	if (m_componentMates != NULL)
//	{
//		componentMatesArray[i]->AddRef();

//		m_componentMates->get_Name2(&name);
//		//m_childComponent->GetMaterialIdName(&material);
//		//m_childComponent->get_ReferencedConfiguration(&cfg);
//		printf("%S \n", name);
//		printf(" \n");
//		TraverseComponents(m_childComponent, level + 1);

//	}

//	m_componentMates = NULL;
//}

//void TraverseModelFeatures(IModelDoc2** swModel, long nLevel) {
//		CComPtr<IFeature> swFeat;
//		/*IFeature* swFeat;*/
//		(*swModel)->IFirstFeature(&swFeat);
//		
//			
//
//		TraverseFeatureFeatures(swFeat, nLevel);
//		
//
//	}




//void TraverseMateFeatures(IFeature* swFeat, long nLevel) {
//		HRESULT hres;
//		/*string sPadStr;*/
//		//long i;
//
//		CComPtr<IFeature>  strFeature;
//
//		strFeature = swFeat;
//
//		CComPtr<IFeature>  swSubFeature;
//
//		CComBSTR  strFeatureName;
//		CComBSTR  strFeatureType;
//
//		CComBSTR  strSubFeatureName;
//		CComBSTR  strSubFeatureType;
//
//		IFeature* featuretype;
//		IFeature* featureName;
//
//		CComPtr<IFeature> swMateFeature;
//		CComPtr<IFeature> swSubMateFeature;
//		//IDispatch *pDisp;
//		//CComBSTR  strMateFeatureName;
//		//CComBSTR  strMateFeatureType;
//		//IMate2* swMate;
//		//
//
//		//LPDISPATCH* componentChildrenArray;
//		//VARIANT vChildren;
//		//HRESULT hres = recComp->GetChildren(&vChildren);
//		//SAFEARRAY* psa = V_ARRAY(&vChildren);
//		//HRESULT res = SafeArrayAccessData(psa, (void **)&componentChildrenArray);
//		//LPCOMPONENT2 m_childComponent = NULL;
//
//		IMate2* swMate;
//		IDispatch *MateArray;
//		IMateEntity2** LpMateEntity0;
//		IMateEntity2** LpMateEntity1;
//		
//
//		//for (i = 0; i < nLevel; ++i) {
//		//	sPadStr = sPadStr + string(" ");
//		//}
//		while (strFeature) {
//
//			strFeature->GetTypeName2(&strFeatureType);
//
//			if (strFeatureType == "MateGroup") {
//				swMateFeature = strFeature;
//				break;
//			}
//
//			hres = strFeature->
//				IGetNextFeature(&strFeature);
//		}
//
//		hres = swMateFeature->IGetFirstSubFeature(&swSubFeature);
//			
//
//			//strFeature->IGetFirstSubFeature(&swSubFeature);
//
//
//			while (swSubFeature) {
//
//				hres= swSubFeature->GetSpecificFeature2(&MateArray);
//
//				MateArray->QueryInterface(CLSID_Mate2, (LPVOID*)&swMate);
//
//				if (swMate != NULL) {
//					BSTR MateName, Component1, Component2;
//					long MateCount;
//
//					
//					hres = swMate->GetMateEntityCount(&MateCount);
//
//					printf("The number of mates is %ld", MateCount);
//					hres = swSubMateFeature->get_Name(&MateName);
//				/*	hres = swMate->MateEntity(0, LpMateEntity0);
//					hres = swMate->MateEntity(1, LpMateEntity1);*/
//					
//					
//
//
//					//string MateName = swSubMateFeature.Name;
//					//string Component1 = swMate.MateEntity(0).ReferenceComponent.Name2;
//					//string Component2 = swMate.MateEntity(1).ReferenceComponent.Name2;
//				}
//
//	
//
//				CComPtr<IFeature>  swNextSubFeature;
//
//
//
//				swSubFeature->IGetNextSubFeature(&swNextSubFeature);
//
//
//
//				swSubFeature = swNextSubFeature;
//
//
//
//
//			}
//
//
//
//
//
//
//}

/*while (swFeature) {

swFeature->get_Name(&strFeatureName);

swFeature->GetTypeName2(&strFeatureType);

printf("%S               %S\n", (wchar_t*)strFeatureName, (wchar_t*)strFeatureType);

swFeature->IGetFirstSubFeature(&swSubFeature);

while (swSubFeature) {

swSubFeature->get_Name(&strFeatureName);
swSubFeature->GetTypeName2(&strFeatureType);

printf("sub         %S               %S\n", (wchar_t*)strFeatureName, (wchar_t*)strFeatureType);

CComPtr<IFeature>  swNextSubFeature;
swSubFeature->IGetNextSubFeature(&swNextSubFeature);
swSubFeature = swNextSubFeature;

}

CComPtr<IFeature>  swNextFeature;
swFeature->IGetNextFeature(&swNextFeature);
swFeature = swNextFeature;

}*/

	

	







