#include "Shader.h"

ShaderFX::ShaderFX(std::string effectPath, ID3D10Device1* pD3DDevice){

	m_pD3DDevice = pD3DDevice;	
	m_effectPath = effectPath;
}

ShaderFX::~ShaderFX(){}


void ShaderFX::createEffect(){

	if (FAILED(D3DX10CreateEffectFromFile(m_effectPath.c_str(), NULL, NULL, "fx_4_0", D3D10_SHADER_ENABLE_STRICTNESS, 0, m_pD3DDevice, NULL, NULL, &m_pEffect, NULL, NULL))){

		std::cout << "Could not create effect!" << std::endl;
	}
}

void ShaderFX::loadTechniquePointer(std::string techniqueName){

	m_pTechnique = m_pEffect->GetTechniqueByName(techniqueName.c_str());
	if (m_pTechnique == NULL){
		std::cout << "Could not find technique with name: " << techniqueName << std::endl;
	}
}

void ShaderFX::createInputLayout(){

	D3D10_INPUT_ELEMENT_DESC inputElementDesc[] = {

		{ "POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0, D3D10_INPUT_PER_VERTEX_DATA, 0 },
		{ "TEXCOORD", 0, DXGI_FORMAT_R32G32_FLOAT, 0, 12, D3D10_INPUT_PER_VERTEX_DATA, 0 }
	};

	int numElements = sizeof(inputElementDesc) / sizeof(inputElementDesc[0]);
	//create input layout
	D3D10_PASS_DESC PassDesc;
	m_pTechnique->GetPassByIndex(0)->GetDesc(&PassDesc);

	if (FAILED(m_pD3DDevice->CreateInputLayout(inputElementDesc, numElements, PassDesc.pIAInputSignature, PassDesc.IAInputSignatureSize, &m_pInputLayout)))
		std::cout << "Could not create Input Layout!" << std::endl;

	//get technique description
	m_pTechnique->GetDesc(&m_techDesc);

}

void ShaderFX::createInputLayout(const D3D10_INPUT_ELEMENT_DESC *pInputElementDescs, const int numInputLayoutElements){

	//create input layout
	D3D10_PASS_DESC PassDesc;
	m_pTechnique->GetPassByIndex(0)->GetDesc(&PassDesc);

	if (FAILED(m_pD3DDevice->CreateInputLayout(&pInputElementDescs[0], numInputLayoutElements, PassDesc.pIAInputSignature, PassDesc.IAInputSignatureSize, &m_pInputLayout)))
		std::cout << "Could not create Input Layout!" << std::endl;

	//get technique description
	m_pTechnique->GetDesc(&m_techDesc);
}

void ShaderFX::createInputLayoutDescFromVertexShaderSignature(){

	ID3D10Blob* errorMessage = 0;
	ID3D10Blob* vertexShaderBuffer = 0;
	
	if (FAILED(D3DX10CompileFromFile(m_effectPath.c_str(), NULL, NULL, "VS", "vs_4_0", D3D10_SHADER_ENABLE_STRICTNESS, 0, 0, &vertexShaderBuffer, &errorMessage, 0))){

		if (errorMessage != NULL){

			std::cout << "Could not compile vertex shader: " << std::endl;
			errorMessage->Release();
		}
		return;
	}

	// Reflect shader info
	ID3D10ShaderReflection* pVertexShaderReflection = NULL;
	if (FAILED(D3D10ReflectShader(vertexShaderBuffer->GetBufferPointer(), vertexShaderBuffer->GetBufferSize(), &pVertexShaderReflection))){
		
		std::cout << "Could not reflect vertexshader " << std::endl;
	}

	// Get shader info
	D3D10_SHADER_DESC shaderDesc;
	pVertexShaderReflection->GetDesc(&shaderDesc);

	// Read input layout description from shader info
	std::vector<D3D10_INPUT_ELEMENT_DESC> inputLayoutDesc;
	for (int i = 0; i< shaderDesc.InputParameters; i++){

		D3D10_SIGNATURE_PARAMETER_DESC paramDesc;
		pVertexShaderReflection->GetInputParameterDesc(i, &paramDesc);

		// fill out input element desc
		D3D10_INPUT_ELEMENT_DESC elementDesc;
		elementDesc.SemanticName = paramDesc.SemanticName;
		elementDesc.SemanticIndex = paramDesc.SemanticIndex;
		elementDesc.InputSlot = 0;
		elementDesc.AlignedByteOffset = D3D10_APPEND_ALIGNED_ELEMENT;
		elementDesc.InputSlotClass = D3D10_INPUT_PER_VERTEX_DATA;
		elementDesc.InstanceDataStepRate = 0;

		// determine DXGI format
		if (paramDesc.Mask == 1){

			if (paramDesc.ComponentType == D3D_REGISTER_COMPONENT_UINT32) elementDesc.Format = DXGI_FORMAT_R32_UINT;
			else if (paramDesc.ComponentType == D3D_REGISTER_COMPONENT_SINT32) elementDesc.Format = DXGI_FORMAT_R32_SINT;
			else if (paramDesc.ComponentType == D3D_REGISTER_COMPONENT_FLOAT32) elementDesc.Format = DXGI_FORMAT_R32_FLOAT;

		}else if (paramDesc.Mask <= 3){

			if (paramDesc.ComponentType == D3D_REGISTER_COMPONENT_UINT32) elementDesc.Format = DXGI_FORMAT_R32G32_UINT;
			else if (paramDesc.ComponentType == D3D_REGISTER_COMPONENT_SINT32) elementDesc.Format = DXGI_FORMAT_R32G32_SINT;
			else if (paramDesc.ComponentType == D3D_REGISTER_COMPONENT_FLOAT32) elementDesc.Format = DXGI_FORMAT_R32G32_FLOAT;

		}else if (paramDesc.Mask <= 7){
			
			if (paramDesc.ComponentType == D3D_REGISTER_COMPONENT_UINT32) elementDesc.Format = DXGI_FORMAT_R32G32B32_UINT;
			else if (paramDesc.ComponentType == D3D_REGISTER_COMPONENT_SINT32) elementDesc.Format = DXGI_FORMAT_R32G32B32_SINT;
			else if (paramDesc.ComponentType == D3D_REGISTER_COMPONENT_FLOAT32) elementDesc.Format = DXGI_FORMAT_R32G32B32_FLOAT;

		}else if (paramDesc.Mask <= 15){
			
			if (paramDesc.ComponentType == D3D_REGISTER_COMPONENT_UINT32) elementDesc.Format = DXGI_FORMAT_R32G32B32A32_UINT;
			else if (paramDesc.ComponentType == D3D_REGISTER_COMPONENT_SINT32) elementDesc.Format = DXGI_FORMAT_R32G32B32A32_SINT;
			else if (paramDesc.ComponentType == D3D_REGISTER_COMPONENT_FLOAT32) elementDesc.Format = DXGI_FORMAT_R32G32B32A32_FLOAT;
		}

		//save element desc
		inputLayoutDesc.push_back(elementDesc);
	}

	D3D10_PASS_DESC PassDesc;
	m_pTechnique->GetPassByIndex(0)->GetDesc(&PassDesc);
	// Try to create Input Layout
	if (FAILED(m_pD3DDevice->CreateInputLayout(&inputLayoutDesc[0], inputLayoutDesc.size(), PassDesc.pIAInputSignature, PassDesc.IAInputSignatureSize, &m_pInputLayout))){

		std::cout << "Could not create Input Layout!" << std::endl;
	}
	//get technique description
	m_pTechnique->GetDesc(&m_techDesc);

	return;
}


void ShaderFX::loadMatrix(const char* location, const D3DXMATRIXA16& matrix){

	m_pEffect->GetVariableByName(location)->AsMatrix()->SetMatrix((float*)&matrix[0]);

}

void ShaderFX::setTexture(const char* location, const ID3D10ShaderResourceView& texture){

	m_pEffect->GetVariableByName(location)->AsShaderResource()->SetResource((ID3D10ShaderResourceView*)&texture);
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

Shader::Shader(ID3D10Device1* pD3DDevice){

	m_pD3DDevice = pD3DDevice;
}

Shader::~Shader(){}

void Shader::compileFromFile(std::string vertexShader, std::string pixelShader, ID3D10Device1* pD3DDevice){

	
	ID3D10Blob* errorMessage;

	// Initialize the pointers this function will use to null.
	errorMessage = 0;
	m_vertexShaderBuffer = 0;
	m_pixelShaderBuffer = 0;

	if (FAILED(D3DX10CompileFromFile(vertexShader.c_str(), NULL, NULL, "VS", "vs_4_0", D3D10_SHADER_ENABLE_STRICTNESS, 0, 0, &m_vertexShaderBuffer, &errorMessage, 0))){

		if (errorMessage != NULL){

			std::cout << "Could not compile vertex shader: " << std::endl;
			errorMessage->Release();
		}
		return;
	}

	pD3DDevice->CreateVertexShader(m_vertexShaderBuffer->GetBufferPointer(), m_vertexShaderBuffer->GetBufferSize(), &m_vertexShader);

	if (FAILED(D3DX10CompileFromFile(pixelShader.c_str(), NULL, NULL, "PS", "ps_4_0", D3D10_SHADER_ENABLE_STRICTNESS, 0, 0, &m_pixelShaderBuffer, &errorMessage, 0))){

		if (errorMessage != NULL){

			std::cout << "Could not compile pixel shader: " << std::endl;
			errorMessage->Release();
		}
		return;

	}

	pD3DDevice->CreatePixelShader(m_pixelShaderBuffer->GetBufferPointer(), m_pixelShaderBuffer->GetBufferSize(), &m_pixelShader);

}

void Shader::createInputLayoutDescFromVertexShaderSignature(){
	
	// Reflect shader info
	ID3D10ShaderReflection* pVertexShaderReflection = NULL;
	if (FAILED(D3D10ReflectShader(m_vertexShaderBuffer->GetBufferPointer(), m_vertexShaderBuffer->GetBufferSize(), &pVertexShaderReflection))){
		
		std::cout << "Could not reflect vertexshader " << std::endl;
	}

	// Get shader info
	D3D10_SHADER_DESC shaderDesc;
	pVertexShaderReflection->GetDesc(&shaderDesc);

	// Read input layout description from shader info
	std::vector<D3D10_INPUT_ELEMENT_DESC> inputLayoutDesc;
	for (int i = 0; i< shaderDesc.InputParameters; i++){

		D3D10_SIGNATURE_PARAMETER_DESC paramDesc;
		pVertexShaderReflection->GetInputParameterDesc(i, &paramDesc);

		// fill out input element desc
		D3D10_INPUT_ELEMENT_DESC elementDesc;
		elementDesc.SemanticName = paramDesc.SemanticName;
		elementDesc.SemanticIndex = paramDesc.SemanticIndex;
		elementDesc.InputSlot = 0;
		elementDesc.AlignedByteOffset = D3D10_APPEND_ALIGNED_ELEMENT;
		elementDesc.InputSlotClass = D3D10_INPUT_PER_VERTEX_DATA;
		elementDesc.InstanceDataStepRate = 0;

		// determine DXGI format
		if (paramDesc.Mask == 1){

			if (paramDesc.ComponentType == D3D_REGISTER_COMPONENT_UINT32) elementDesc.Format = DXGI_FORMAT_R32_UINT;
			else if (paramDesc.ComponentType == D3D_REGISTER_COMPONENT_SINT32) elementDesc.Format = DXGI_FORMAT_R32_SINT;
			else if (paramDesc.ComponentType == D3D_REGISTER_COMPONENT_FLOAT32) elementDesc.Format = DXGI_FORMAT_R32_FLOAT;

		}else if (paramDesc.Mask <= 3){

			if (paramDesc.ComponentType == D3D_REGISTER_COMPONENT_UINT32) elementDesc.Format = DXGI_FORMAT_R32G32_UINT;
			else if (paramDesc.ComponentType == D3D_REGISTER_COMPONENT_SINT32) elementDesc.Format = DXGI_FORMAT_R32G32_SINT;
			else if (paramDesc.ComponentType == D3D_REGISTER_COMPONENT_FLOAT32) elementDesc.Format = DXGI_FORMAT_R32G32_FLOAT;

		}else if (paramDesc.Mask <= 7){

			if (paramDesc.ComponentType == D3D_REGISTER_COMPONENT_UINT32) elementDesc.Format = DXGI_FORMAT_R32G32B32_UINT;
			else if (paramDesc.ComponentType == D3D_REGISTER_COMPONENT_SINT32) elementDesc.Format = DXGI_FORMAT_R32G32B32_SINT;
			else if (paramDesc.ComponentType == D3D_REGISTER_COMPONENT_FLOAT32) elementDesc.Format = DXGI_FORMAT_R32G32B32_FLOAT;

		}else if (paramDesc.Mask <= 15){

			if (paramDesc.ComponentType == D3D_REGISTER_COMPONENT_UINT32) elementDesc.Format = DXGI_FORMAT_R32G32B32A32_UINT;
			else if (paramDesc.ComponentType == D3D_REGISTER_COMPONENT_SINT32) elementDesc.Format = DXGI_FORMAT_R32G32B32A32_SINT;
			else if (paramDesc.ComponentType == D3D_REGISTER_COMPONENT_FLOAT32) elementDesc.Format = DXGI_FORMAT_R32G32B32A32_FLOAT;
		}

		//save element desc
		inputLayoutDesc.push_back(elementDesc);
	}

	// Try to create Input Layout
	if (FAILED(m_pD3DDevice->CreateInputLayout(&inputLayoutDesc[0], inputLayoutDesc.size(), m_vertexShaderBuffer->GetBufferPointer(), m_vertexShaderBuffer->GetBufferSize(), &m_pInputLayout))){
		
		std::cout << "Could not create Input Layout!" << std::endl;
	}

	//Free allocation shader reflection memory
	pVertexShaderReflection->Release();
	pVertexShaderReflection = 0;

}



void Shader::createConstBuffer(){

	// Setup the description of the dynamic matrix constant buffer that is in the vertex shader.
	D3D10_BUFFER_DESC matrixBufferDesc;
	matrixBufferDesc.Usage = D3D10_USAGE_DYNAMIC;
	matrixBufferDesc.ByteWidth = sizeof(MatrixBuffer);
	matrixBufferDesc.BindFlags = D3D10_BIND_CONSTANT_BUFFER;
	matrixBufferDesc.CPUAccessFlags = D3D10_CPU_ACCESS_WRITE;
	matrixBufferDesc.MiscFlags = 0;

	// Create the constant buffer pointer so we can access the vertex shader constant buffer from within this class.
	if (FAILED(m_pD3DDevice->CreateBuffer(&matrixBufferDesc, NULL, &m_matrixBuffer))){

		std::cout << "Could not create constant buffer!" << std::endl;
	}
}

void Shader::bindShader(){

	m_pD3DDevice->VSSetShader(m_vertexShader);
	m_pD3DDevice->PSSetShader(m_pixelShader);
}

void Shader::loadMatrix(Location location, const D3DXMATRIXA16& matrix){

	/*switch (location){

	case Model:
	m_matrixBuffer.model = matrix;
	case View:
	m_matrixBuffer.view = matrix;
	case Projection:
	m_matrixBuffer.proj = matrix;

	}*/
	//m_matrixBuffer.mvp = matrix;
}


bool Shader::SetShaderParameters(const D3DXMATRIXA16& worldMatrix, const D3DXMATRIXA16& viewMatrix, const D3DXMATRIXA16& projectionMatrix){

	HRESULT result;
	void* mappedResource;
	MatrixBuffer* dataPtr;
	unsigned int bufferNumber;

	D3DXMATRIXA16 projTranspose;
	D3DXMatrixTranspose(&projTranspose, &projectionMatrix);

	D3DXMATRIXA16 worldTranspose;
	D3DXMatrixTranspose(&worldTranspose, &worldMatrix);

	D3DXMATRIXA16 viewTranspose;
	D3DXMatrixTranspose(&viewTranspose, &viewMatrix);


	// Lock the constant buffer so it can be written to.
	result = m_matrixBuffer->Map(D3D10_MAP_WRITE_DISCARD, 0, &mappedResource);
	if (FAILED(result)){

		std::cout << "could not update const buffer" << std::endl;

		return false;
	}

	// Get a pointer to the data in the constant buffer.
	dataPtr = (MatrixBuffer*)mappedResource;

	// Copy the matrices into the constant buffer.
	dataPtr->world = worldTranspose;
	dataPtr->view = viewTranspose;
	dataPtr->projection = projTranspose;

	// Unlock the constant buffer.
	m_matrixBuffer->Unmap();

	// Set the position of the constant buffer in the vertex shader.
	bufferNumber = 0;

	// Finanly set the constant buffer in the vertex shader with the updated values.
	m_pD3DDevice->VSSetConstantBuffers(bufferNumber, 1, &m_matrixBuffer);

	return true;
}

