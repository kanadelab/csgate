
//�A���}�l�[�W�h�Ńr���h�B
//���SAORI���o��
#pragma unmanaged
#include <Windows.h>

BOOL WINAPI DllMain(HANDLE hModule, DWORD, LPVOID)
{
	return TRUE;
}



//��������̓}�l�[�W�h�r���h�B
#pragma managed
using namespace System;
using namespace System::Collections::Generic;
using namespace System::Reflection;
#include<gcroot.h>

void MainInternal()
{
	System::Windows::Forms::MessageBox::Show("AAA");
}

int main(void)
{
	MainInternal();
}



public ref class SaoriAsm
{
public:
	static property Dictionary<String^, Assembly^>^ AsmDic;
	static property Microsoft::CSharp::CSharpCodeProvider^ CodeProvider;
	static property System::CodeDom::Compiler::CompilerParameters^ CompileParam;
	static property String^ CurrentDirectory;
};



HGLOBAL RequestInternal(HGLOBAL h, long* len);


void Initialize(HGLOBAL h, long len)
{

	System::String^ saori_d = gcnew System::String((char*)h, 0, len, System::Text::Encoding::GetEncoding("Shift_JIS"));
	System::String^ saori_dir = gcnew System::String(saori_d);
	SaoriAsm::CurrentDirectory = saori_dir + "\\";

	//HGLOBAL�̉��
	GlobalFree(h);

	System::Collections::Generic::Dictionary < System::String^, System::String^ >^ options = gcnew System::Collections::Generic::Dictionary < System::String^, System::String^ >();
	options->Add("CompilerVersion", "v3.5");

	SaoriAsm::AsmDic = gcnew Dictionary<String^, Assembly^>();
	SaoriAsm::CodeProvider = gcnew Microsoft::CSharp::CSharpCodeProvider(options);
	SaoriAsm::CompileParam = gcnew System::CodeDom::Compiler::CompilerParameters();


	SaoriAsm::CompileParam->GenerateInMemory = true;
	SaoriAsm::CompileParam->IncludeDebugInformation = false;
}



String^ ExecuteSaori(Dictionary<String^, String^>^ request_dic);
HGLOBAL RequestInternal(HGLOBAL h, long* len)
{
	//�������炪��肾�B���N�G�X�g����͂��˂΁B
	System::String^ req = gcnew System::String((char*)h, 0, *len, System::Text::Encoding::GetEncoding("Shift_JIS"));
	GlobalFree(h);

	System::IO::StringReader^ read = gcnew System::IO::StringReader(req);
	Dictionary<String^, String^>^ request_dic = gcnew Dictionary<String^, String^>();

	array<String^>^ splitter = { ": " };
	String^ head = nullptr;

	while (true)
	{
		System::String^ line = read->ReadLine();
		if (line == nullptr)
		{
			break;
		}

		if (head == nullptr)
		{
			head = line;
			continue;
		}

		array<String^>^ sp = line->Split(splitter, 2, StringSplitOptions::None);
		if (sp->Length == 2)
		{
			request_dic->Add(sp[0], sp[1]);
		}
	}

	//�����쐬
	String^ ret = "";
	if (head != nullptr)
	{
		if (head->IndexOf("GET Version") == 0)
		{
			ret = "SAORI/1.0 200 OK\r\nCharset: Shift_JIS\r\n\r\n";
		}
		else if (head->IndexOf("EXECUTE") == 0)
		{
			//execute�Ăяo��
			ret = ExecuteSaori(request_dic);
		}
		else
		{
			//bad
			ret = "SAORI/1.0 400 Bad Request\r\nCharset: Shift_JIS\r\n\r\n";
		}
	}
	else
	{
		ret = "SAORI/1.0 400 Bad Request\r\nCharset: Shift_JIS\r\n\r\n";
	}

	//�ԑ�
	array<unsigned char, 1>^ ar = System::Text::Encoding::GetEncoding("Shift_JIS")->GetBytes(ret);
	HGLOBAL hg = GlobalAlloc(GMEM_FIXED, ar->Length);
	System::Runtime::InteropServices::Marshal::Copy(ar, 0, System::IntPtr(hg), ar->Length);

	*len = ar->Length;
	return hg;
}

bool Compile(String^ key, array<String^>^ files);
String^ ExecuteSaori(Dictionary<String^, String^>^ request_dic)
{
	//args��́i�A������arg�݂̂��m�F����j
	List<String^>^ argList = gcnew List<String^>();
	while (true)
	{
		String^ key = "Argument" + argList->Count.ToString();
		if (request_dic->ContainsKey(key))
		{
			argList->Add(request_dic[key]);
		}
		else
		{
			break;
		}
	}

	//��͂���args��U��B
	
	
	
	if (argList[0] == "add_reference")
	{
		//�Q�ƃA�Z���u���̒ǉ�
		for (Int32 i = 1; i < argList->Count; i++)
		{
			if (!SaoriAsm::CompileParam->ReferencedAssemblies->Contains(argList[i]))
			{
				SaoriAsm::CompileParam->ReferencedAssemblies->Add(argList[i]);
			}
		}
	}
	else if (argList[0] == "remove_reference")
	{
		//�Q�ƃA�Z���u���̏��O
		for (Int32 i = 1; i < argList->Count; i++)
		{
			if (SaoriAsm::CompileParam->ReferencedAssemblies->Contains(argList[i]))
			{
				SaoriAsm::CompileParam->ReferencedAssemblies->Remove(argList[i]);
			}
		}
	}
	else if (argList[0] == "compile_assembly")
	{
		//�A�Z���u���R���p�C��
		List<String^>^ files = gcnew List<String^>();
		for (Int32 i = 2; i < argList->Count; i++)
		{
			files->Add(SaoriAsm::CurrentDirectory +  argList[i]);
		}

		Compile(argList[1], files->ToArray());
	}
	else if (argList[0] == "call_function")
	{
		//�A�Z���u���̊֐��Ăяo��
		if (SaoriAsm::AsmDic->ContainsKey(argList[1]))
		{
			Assembly^ a = SaoriAsm::AsmDic[argList[1] ];
			System::Type^ t = a->GetType(argList[2]);
			if (t != nullptr)
			{
				System::Reflection::MethodInfo^ m = t->GetMethod(argList[3]);
				if (m != nullptr)
				{

					array<Object^>^ args_array = argList->GetRange(4, argList->Count - 4)->ToArray();
					try
					{
						m->Invoke(nullptr, args_array);
					}
					catch (System::Reflection::TargetInvocationException^ e)
					{
						System::Console::WriteLine(e->InnerException->Message);
					}

					String^ res_str = nullptr;
					String^ ret_result = "";
					List<String^>^ ret_value = gcnew List<String^>();
					System::Collections::Generic::IEnumerable<String^>^ val_str = nullptr;

					//�Ăяo�����N���X�ɐÓI�v���p�e�B Result,Value�������SAORI�̖߂�l�Ƃ��Ďg�p
					PropertyInfo^ result_info = t->GetProperty("Result");
					if (result_info != nullptr)
					{
						Object^ res = result_info->GetValue(nullptr, nullptr);
						res_str = dynamic_cast<String^>(res);
					}

					PropertyInfo^ value_info = t->GetProperty("Value");
					if (value_info != nullptr)
					{
						Object^ val = value_info->GetValue(nullptr, nullptr);
						val_str = dynamic_cast<System::Collections::Generic::IEnumerable<String^>^>(val);
					}

					if (res_str != nullptr)
					{
						ret_result = res_str;
					}

					if (val_str != nullptr)
					{
						ret_value->AddRange(val_str);
					}

					String^ res = "SAORI/1.0 200 OK\r\nCharset: Shift_JIS\r\nResult: " + ret_result + "\r\n";
					for (Int32 i = 0; i <ret_value->Count; i++)
					{
						res += "Value" + i.ToString() + ": " + ret_value[i] + "\r\n";
					}

					res += "\r\n";


					return res;
				}
			}
		}
	}


	//���ɖ��������炻��ł�����
	return "SAORI/1.0 200 OK\r\nCharset: Shift_JIS\r\nResult: \r\n\r\n";

}

bool Compile( String^ key, array<String^>^ files )
{
	if (SaoriAsm::AsmDic->ContainsKey(key))
	{
		return false;
	}

	System::CodeDom::Compiler::CompilerResults^ result = SaoriAsm::CodeProvider->CompileAssemblyFromFile(SaoriAsm::CompileParam, files);	//�r���h����t�@�C����
	
	if (result->Errors->Count == 0)
	{
		//�R���p�C�������I
		SaoriAsm::AsmDic->Add(key, result->CompiledAssembly);
		return true;
	}
	else
	{
		//�R���p�C���G���[����
		String^ err = "";
		for (int i = 0; i < result->Errors->Count; i++)
		{
			System::CodeDom::Compiler::CompilerError^ e = result->Errors[i];
			err += e->ToString() + "\r\n";
		}

		System::Windows::Forms::MessageBox::Show(err);
		return false;
	}


}


//������̌Ăяo�����̓A���}�l�[�W�Ńr���h
#pragma unmanaged
extern "C" __declspec(dllexport) BOOL __cdecl load(HGLOBAL h, long len)
{
	Initialize(h, len);
	return TRUE;
}

extern "C" __declspec(dllexport) HGLOBAL __cdecl request(HGLOBAL h, long* len)
{
	return RequestInternal(h, len);
}


extern "C" __declspec(dllexport) BOOL __cdecl unload()
{
	return TRUE;
}