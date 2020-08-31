#pragma once

namespace CppCLRWinformsProjekt {
	ref class Form1;

	class Average
	{
	public:
		float comps[3];

		void Zero()
		{
			comps[0] = 0;
			comps[1] = 0;
			comps[2] = 0;
		}

		void Add(Average* avg)
		{
			this->comps[0] += avg->comps[0];
			this->comps[1] += avg->comps[1];
			this->comps[2] += avg->comps[2];
		}

		void Add(int x, int y, unsigned char* bmporg, int w, int h)
		{
			if (x < 0 || y < 0 || x >= w || y >= h)
			{
				//comps[0] += 255;
				//comps[1] += 255;
				//comps[2] += 255;
				return;
			}

			x += y * w;
			x *= 3;
			comps[0] += bmporg[x + 0];
			comps[1] += bmporg[x + 1];
			comps[2] += bmporg[x + 2];
		}

		void Sub(int x, int y, unsigned char* bmporg, int w, int h)
		{
			if (x < 0 || y < 0 || x >= w || y >= h)
			{
				//comps[0] -= 255;
				//comps[1] -= 255;
				//comps[2] -= 255;
				return;
			}

			x += y * w;
			x *= 3;
			comps[0] -= bmporg[x + 0];
			comps[1] -= bmporg[x + 1];
			comps[2] -= bmporg[x + 2];
		}

		void Multiply(float m)
		{
			comps[0] *= m;
			comps[1] *= m;
			comps[2] *= m;
		}

		void WritePixel(int a, int b, unsigned char* bmpDest, int w, int h)
		{
			if (a < 0 || b < 0 || a >= w || b >= h)
			{
				return;
			}
			a += b * w;
			a *= 3;

			bmpDest[a + 0] = (unsigned char)comps[0];
			bmpDest[a + 1] = (unsigned char)comps[1];
			bmpDest[a + 2] = (unsigned char)comps[2];
		}
	};
	using namespace System;
	using namespace System::ComponentModel;
	using namespace System::Collections;
	using namespace System::Windows::Forms;
	using namespace System::Data;
	using namespace System::Drawing;
	using namespace System::Drawing::Imaging;
	/// <summary>
	/// Zusammenfassung f�r Form1
	/// </summary>
	public ref class Form1 : public System::Windows::Forms::Form
	{
	public:
		Bitmap^ bmpFront;
		unsigned char* bmpOriginal;
		unsigned char* hBlur;
		static int imgSizeInBytes = -1;
		static Rectangle imgRect;
		BitmapData^ bmpData;

		static double Brightness_cppCount = 0.0;
		static double Brightness_cppTotal = 0.0;
		static double Brightness_asmCount = 0.0;
		static double Brightness_asmTotal = 0.0;
		static double Negative_asmCount = 0.0;
		static double Negative_asmTotal = 0.0;
		static double Negative_cppTotal = 0.0;
		static double Negative_cppCount = 0.0;
		static double Blur_asmTotal = 0.0;
		static double Blur_asmCount = 0.0;
		static double Blur_cppTotal = 0.0;
		static double Blur_cppCount = 0.0;

		void AdjustBrightness(unsigned char* bmp, short amount)
		{
			for (int i = 0; i < imgSizeInBytes; i++)
			{
				if ((short)bmpOriginal[i] + amount < 0) bmp[i] = 0;
				else if ((short)bmpOriginal[i] + amount > 255) bmp[i] = 255;
				else bmp[i] = bmpOriginal[i] + amount;
			}
		}

		void CPPNegativeIMG(unsigned char* bmp, unsigned char* original,int imgSize,bool isChecked)
		{	
			if(isChecked)
			{
				for (int i = 0; i < imgSizeInBytes; i++)
				{
					bmp[i] = 255 - original[i];
				}
			}
			else
			{
				for (int i = 0; i < imgSizeInBytes; i++)
				{
					bmp[i] = original[i];
				}
			}
		}

		void CPPBlurIMG(unsigned char* bmp, short blurWidth)
		{
			int height, width;
			Average avg,tmp;
			float rcpboxWidth = (1.0f) / (blurWidth * 2 + 1);

			height = bmpFront->Height;
			width = bmpFront->Width;
			hBlur = new unsigned char [imgSizeInBytes];

			//Horizontal blur
			for(int y = 0; y<height ;y++)
			{
				avg.Zero();
				for(int x = -blurWidth-1;x<blurWidth;x++)
				{	
					avg.Add(x, y, bmpOriginal, width, height);
					
				}
				avg.Multiply(rcpboxWidth);

				for (int x = 0; x < width; x++)
				{
					tmp.Zero();
					tmp.Sub(x - blurWidth - 1, y, bmpOriginal, width, height);
					tmp.Add(x + blurWidth, y, bmpOriginal, width, height);

					tmp.Multiply(rcpboxWidth);

					avg.Add(&tmp);

					avg.WritePixel(x, y, hBlur, width, height);
				}

			}
			//Vertical blur
			for (int y = 0; y < width; y++)
			{
				avg.Zero();
				for (int x = -blurWidth - 1; x < blurWidth; x++)
				{
					avg.Add(y, x, hBlur, width, height);
				}

				avg.Multiply(rcpboxWidth);

				for (int x = 0; x < height; x++)
				{
					tmp.Zero();
					tmp.Sub(y, x - blurWidth - 1, hBlur, width, height);
					tmp.Add(y , x + blurWidth, hBlur, width, height);

					tmp.Multiply(rcpboxWidth);

					avg.Add(&tmp);

					avg.WritePixel(y, x, bmp, width, height);
				}

			}
		}

		void ClearOriginalImage()
		{
			if (bmpOriginal != nullptr) { delete[] bmpOriginal; }
		}

		//Make a copy of the original image
		void SaveOriginalImage(System::Drawing::Bitmap^ bmp)
		{
			ClearOriginalImage();

			imgSizeInBytes = bmp->Width * bmp->Height * 3;

			bmpOriginal = new unsigned char[imgSizeInBytes];

			imgRect.Width = bmp->Width;
			imgRect.Height = bmp->Height;

			bmpData = bmp->LockBits(imgRect, ImageLockMode::ReadOnly, PixelFormat::Format24bppRgb);

			unsigned char* p = (unsigned char*)bmpData->Scan0.ToPointer();

			for (int i = 0; i < imgSizeInBytes; i++)
			{
				bmpOriginal[i] = *p++;
			}

			bmp->UnlockBits(bmpData);

		}

		Form1(void)
		{
			InitializeComponent();

		}

	protected:
		~Form1()
		{
			if (components)
			{
				delete components;
			}
		}
	private: System::Windows::Forms::MenuStrip^ menuStrip1;
	protected:
	private: System::Windows::Forms::ToolStripMenuItem^ fileToolStripMenuItem;
	private: System::Windows::Forms::ToolStripMenuItem^ openToolStripMenuItem;
	private: System::Windows::Forms::ToolStripMenuItem^ exitToolStripMenuItem;

	private: System::Windows::Forms::PictureBox^ pictureBoxImg;
	private: System::Windows::Forms::Label^ averageTimeLabel;

	private: System::Windows::Forms::OpenFileDialog^ dlgOpen;

	private: System::Windows::Forms::TrackBar^ brightnessTrackbar;
	private: System::Windows::Forms::Label^ brightnessLabel;

	private: System::Windows::Forms::Label^ optionsLabel;
	private: System::Windows::Forms::CheckBox^ negativeCheckbox;

	private: System::Windows::Forms::TrackBar^ blurTrackbar;
	private: System::Windows::Forms::Label^ blurLabel;

	private: System::Windows::Forms::CheckBox^ ASMCheckBox;


	private: System::ComponentModel::Container^ components;

	#pragma region Windows Form Designer generated code
		   /// <summary>
		   /// Erforderliche Methode f�r die Designerunterst�tzung.
		   /// Der Inhalt der Methode darf nicht mit dem Code-Editor ge�ndert werden.
		   /// </summary>
		   void InitializeComponent(void)
		   {
			   this->menuStrip1 = (gcnew System::Windows::Forms::MenuStrip());
			   this->fileToolStripMenuItem = (gcnew System::Windows::Forms::ToolStripMenuItem());
			   this->openToolStripMenuItem = (gcnew System::Windows::Forms::ToolStripMenuItem());
			   this->exitToolStripMenuItem = (gcnew System::Windows::Forms::ToolStripMenuItem());
			   this->pictureBoxImg = (gcnew System::Windows::Forms::PictureBox());
			   this->averageTimeLabel = (gcnew System::Windows::Forms::Label());
			   this->dlgOpen = (gcnew System::Windows::Forms::OpenFileDialog());
			   this->brightnessTrackbar = (gcnew System::Windows::Forms::TrackBar());
			   this->brightnessLabel = (gcnew System::Windows::Forms::Label());
			   this->optionsLabel = (gcnew System::Windows::Forms::Label());
			   this->negativeCheckbox = (gcnew System::Windows::Forms::CheckBox());
			   this->blurTrackbar = (gcnew System::Windows::Forms::TrackBar());
			   this->blurLabel = (gcnew System::Windows::Forms::Label());
			   this->ASMCheckBox = (gcnew System::Windows::Forms::CheckBox());
			   this->menuStrip1->SuspendLayout();
			   (cli::safe_cast<System::ComponentModel::ISupportInitialize^>(this->pictureBoxImg))->BeginInit();
			   (cli::safe_cast<System::ComponentModel::ISupportInitialize^>(this->brightnessTrackbar))->BeginInit();
			   (cli::safe_cast<System::ComponentModel::ISupportInitialize^>(this->blurTrackbar))->BeginInit();
			   this->SuspendLayout();
			   this->menuStrip1->Items->AddRange(gcnew cli::array< System::Windows::Forms::ToolStripItem^  >(1) { this->fileToolStripMenuItem });
			   this->menuStrip1->Location = System::Drawing::Point(0, 0);
			   this->menuStrip1->Name = L"menuStrip1";
			   this->menuStrip1->Size = System::Drawing::Size(758, 24);
			   this->menuStrip1->TabIndex = 0;
			   this->menuStrip1->Text = L"menuStrip1";
			   this->menuStrip1->ItemClicked += gcnew System::Windows::Forms::ToolStripItemClickedEventHandler(this, &Form1::menuStrip1_ItemClicked);
			   this->fileToolStripMenuItem->DropDownItems->AddRange(gcnew cli::array< System::Windows::Forms::ToolStripItem^  >(2) {
				   this->openToolStripMenuItem,
					   this->exitToolStripMenuItem
			   });
			   this->fileToolStripMenuItem->Name = L"fileToolStripMenuItem";
			   this->fileToolStripMenuItem->Size = System::Drawing::Size(37, 20);
			   this->fileToolStripMenuItem->Text = L"File";
			   this->openToolStripMenuItem->Name = L"openToolStripMenuItem";
			   this->openToolStripMenuItem->Size = System::Drawing::Size(103, 22);
			   this->openToolStripMenuItem->Text = L"&Open";
			   this->openToolStripMenuItem->Click += gcnew System::EventHandler(this, &Form1::openToolStripMenuItem_Click);
			   this->exitToolStripMenuItem->Name = L"exitToolStripMenuItem";
			   this->exitToolStripMenuItem->Size = System::Drawing::Size(103, 22);
			   this->exitToolStripMenuItem->Text = L"&Exit";
			   this->exitToolStripMenuItem->Click += gcnew System::EventHandler(this, &Form1::exitToolStripMenuItem_Click);
			   this->pictureBoxImg->Anchor = static_cast<System::Windows::Forms::AnchorStyles>((((System::Windows::Forms::AnchorStyles::Top | System::Windows::Forms::AnchorStyles::Bottom)
				   | System::Windows::Forms::AnchorStyles::Left)
				   | System::Windows::Forms::AnchorStyles::Right));
			   this->pictureBoxImg->BackColor = System::Drawing::SystemColors::Control;
			   this->pictureBoxImg->Location = System::Drawing::Point(12, 27);
			   this->pictureBoxImg->Name = L"pictureBoxImg";
			   this->pictureBoxImg->Size = System::Drawing::Size(734, 359);
			   this->pictureBoxImg->SizeMode = System::Windows::Forms::PictureBoxSizeMode::Zoom;
			   this->pictureBoxImg->TabIndex = 1;
			   this->pictureBoxImg->TabStop = false;
			   this->averageTimeLabel->Anchor = static_cast<System::Windows::Forms::AnchorStyles>((System::Windows::Forms::AnchorStyles::Bottom | System::Windows::Forms::AnchorStyles::Left));
			   this->averageTimeLabel->AutoSize = true;
			   this->averageTimeLabel->BackColor = System::Drawing::SystemColors::GradientActiveCaption;
			   this->averageTimeLabel->Font = (gcnew System::Drawing::Font(L"Microsoft Sans Serif", 12, System::Drawing::FontStyle::Regular, System::Drawing::GraphicsUnit::Point,
				   static_cast<System::Byte>(0)));
			   this->averageTimeLabel->Location = System::Drawing::Point(9, 593);
			   this->averageTimeLabel->Margin = System::Windows::Forms::Padding(0, 0, 3, 0);
			   this->averageTimeLabel->Name = L"averageTimeLabel";
			   this->averageTimeLabel->Size = System::Drawing::Size(120, 20);
			   this->averageTimeLabel->TabIndex = 3;
			   this->averageTimeLabel->Text = L"Average (ms) : -";
			   this->averageTimeLabel->TextAlign = System::Drawing::ContentAlignment::MiddleLeft;
			   this->averageTimeLabel->Click += gcnew System::EventHandler(this, &Form1::label1_Click);
			   this->dlgOpen->FileName = L"openFileDialog1";
			   this->dlgOpen->Filter = L"JPEG|*.jpg|Bitmap|*.bmp|All Files|*.*";
			   this->brightnessTrackbar->Anchor = static_cast<System::Windows::Forms::AnchorStyles>((System::Windows::Forms::AnchorStyles::Bottom | System::Windows::Forms::AnchorStyles::Left));
			   this->brightnessTrackbar->BackColor = System::Drawing::SystemColors::ControlDarkDark;
			   this->brightnessTrackbar->Enabled = false;
			   this->brightnessTrackbar->Location = System::Drawing::Point(12, 415);
			   this->brightnessTrackbar->Maximum = 255;
			   this->brightnessTrackbar->Minimum = -255;
			   this->brightnessTrackbar->Name = L"brightnessTrackbar";
			   this->brightnessTrackbar->Size = System::Drawing::Size(212, 45);
			   this->brightnessTrackbar->TabIndex = 4;
			   this->brightnessTrackbar->TickFrequency = 16;
			   this->brightnessTrackbar->Scroll += gcnew System::EventHandler(this, &Form1::brightnessTrackbar_Scroll);
			   this->brightnessLabel->Anchor = static_cast<System::Windows::Forms::AnchorStyles>((System::Windows::Forms::AnchorStyles::Bottom | System::Windows::Forms::AnchorStyles::Left));
			   this->brightnessLabel->BackColor = System::Drawing::SystemColors::MenuHighlight;
			   this->brightnessLabel->BorderStyle = System::Windows::Forms::BorderStyle::FixedSingle;
			   this->brightnessLabel->Font = (gcnew System::Drawing::Font(L"Microsoft Sans Serif", 14.25, System::Drawing::FontStyle::Bold, System::Drawing::GraphicsUnit::Point,
				   static_cast<System::Byte>(0)));
			   this->brightnessLabel->ForeColor = System::Drawing::Color::White;
			   this->brightnessLabel->Location = System::Drawing::Point(12, 389);
			   this->brightnessLabel->Name = L"brightnessLabel";
			   this->brightnessLabel->Size = System::Drawing::Size(212, 26);
			   this->brightnessLabel->TabIndex = 10;
			   this->brightnessLabel->Text = L"Brightness";
			   this->brightnessLabel->TextAlign = System::Drawing::ContentAlignment::TopCenter;
			   this->optionsLabel->Anchor = static_cast<System::Windows::Forms::AnchorStyles>((System::Windows::Forms::AnchorStyles::Bottom | System::Windows::Forms::AnchorStyles::Right));
			   this->optionsLabel->BackColor = System::Drawing::SystemColors::MenuHighlight;
			   this->optionsLabel->BorderStyle = System::Windows::Forms::BorderStyle::FixedSingle;
			   this->optionsLabel->Font = (gcnew System::Drawing::Font(L"Microsoft Sans Serif", 14.25, System::Drawing::FontStyle::Bold, System::Drawing::GraphicsUnit::Point,
				   static_cast<System::Byte>(0)));
			   this->optionsLabel->ForeColor = System::Drawing::Color::White;
			   this->optionsLabel->Location = System::Drawing::Point(591, 389);
			   this->optionsLabel->Name = L"optionsLabel";
			   this->optionsLabel->Size = System::Drawing::Size(155, 26);
			   this->optionsLabel->TabIndex = 11;
			   this->optionsLabel->Text = L"Options";
			   this->optionsLabel->TextAlign = System::Drawing::ContentAlignment::TopCenter;
			   this->negativeCheckbox->Anchor = static_cast<System::Windows::Forms::AnchorStyles>((System::Windows::Forms::AnchorStyles::Bottom | System::Windows::Forms::AnchorStyles::Right));
			   this->negativeCheckbox->AutoSize = true;
			   this->negativeCheckbox->Enabled = false;
			   this->negativeCheckbox->Location = System::Drawing::Point(592, 418);
			   this->negativeCheckbox->Name = L"negativeCheckbox";
			   this->negativeCheckbox->Size = System::Drawing::Size(69, 17);
			   this->negativeCheckbox->TabIndex = 12;
			   this->negativeCheckbox->Text = L"Negative";
			   this->negativeCheckbox->UseVisualStyleBackColor = true;
			   this->negativeCheckbox->CheckedChanged += gcnew System::EventHandler(this, &Form1::negativeCheckbox_CheckedChanged);
			   this->blurTrackbar->Anchor = static_cast<System::Windows::Forms::AnchorStyles>((System::Windows::Forms::AnchorStyles::Bottom | System::Windows::Forms::AnchorStyles::Left));
			   this->blurTrackbar->BackColor = System::Drawing::SystemColors::ControlDarkDark;
			   this->blurTrackbar->Enabled = false;
			   this->blurTrackbar->LargeChange = 10;
			   this->blurTrackbar->Location = System::Drawing::Point(12, 499);
			   this->blurTrackbar->Margin = System::Windows::Forms::Padding(3, 0, 3, 3);
			   this->blurTrackbar->Maximum = 100;
			   this->blurTrackbar->Name = L"blurTrackbar";
			   this->blurTrackbar->Size = System::Drawing::Size(212, 45);
			   this->blurTrackbar->TabIndex = 1;
			   this->blurTrackbar->TickFrequency = 10;
			   this->blurTrackbar->Scroll += gcnew System::EventHandler(this, &Form1::blurTrackbar_Scroll);
			   this->blurLabel->Anchor = static_cast<System::Windows::Forms::AnchorStyles>((System::Windows::Forms::AnchorStyles::Bottom | System::Windows::Forms::AnchorStyles::Left));
			   this->blurLabel->BackColor = System::Drawing::SystemColors::MenuHighlight;
			   this->blurLabel->BorderStyle = System::Windows::Forms::BorderStyle::FixedSingle;
			   this->blurLabel->Font = (gcnew System::Drawing::Font(L"Microsoft Sans Serif", 14.25, System::Drawing::FontStyle::Bold, System::Drawing::GraphicsUnit::Point,
				   static_cast<System::Byte>(0)));
			   this->blurLabel->ForeColor = System::Drawing::Color::White;
			   this->blurLabel->Location = System::Drawing::Point(12, 473);
			   this->blurLabel->Name = L"blurLabel";
			   this->blurLabel->Size = System::Drawing::Size(212, 26);
			   this->blurLabel->TabIndex = 14;
			   this->blurLabel->Text = L"Blur Width";
			   this->blurLabel->TextAlign = System::Drawing::ContentAlignment::TopCenter;
			   this->ASMCheckBox->Anchor = static_cast<System::Windows::Forms::AnchorStyles>((System::Windows::Forms::AnchorStyles::Bottom | System::Windows::Forms::AnchorStyles::Left));
			   this->ASMCheckBox->AutoSize = true;
			   this->ASMCheckBox->BackColor = System::Drawing::SystemColors::ActiveCaption;
			   this->ASMCheckBox->Enabled = false;
			   this->ASMCheckBox->Font = (gcnew System::Drawing::Font(L"Microsoft Sans Serif", 11.25, System::Drawing::FontStyle::Regular, System::Drawing::GraphicsUnit::Point,
				   static_cast<System::Byte>(0)));
			   this->ASMCheckBox->Location = System::Drawing::Point(12, 568);
			   this->ASMCheckBox->Name = L"ASMCheckBox";
			   this->ASMCheckBox->Size = System::Drawing::Size(90, 22);
			   this->ASMCheckBox->TabIndex = 15;
			   this->ASMCheckBox->Text = L"Use ASM";
			   this->ASMCheckBox->UseVisualStyleBackColor = false;
			   this->AutoScaleDimensions = System::Drawing::SizeF(6, 13);
			   this->AutoScaleMode = System::Windows::Forms::AutoScaleMode::Font;
			   this->BackColor = System::Drawing::SystemColors::ButtonShadow;
			   this->ClientSize = System::Drawing::Size(758, 622);
			   this->Controls->Add(this->ASMCheckBox);
			   this->Controls->Add(this->blurLabel);
			   this->Controls->Add(this->blurTrackbar);
			   this->Controls->Add(this->negativeCheckbox);
			   this->Controls->Add(this->optionsLabel);
			   this->Controls->Add(this->brightnessLabel);
			   this->Controls->Add(this->brightnessTrackbar);
			   this->Controls->Add(this->averageTimeLabel);
			   this->Controls->Add(this->pictureBoxImg);
			   this->Controls->Add(this->menuStrip1);
			   this->MainMenuStrip = this->menuStrip1;
			   this->Name = L"Form1";
			   this->Text = L"Form1";
			   this->FormClosing += gcnew System::Windows::Forms::FormClosingEventHandler(this, &Form1::Form1_FormClosing);
			   this->Load += gcnew System::EventHandler(this, &Form1::Form1_Load);
			   this->menuStrip1->ResumeLayout(false);
			   this->menuStrip1->PerformLayout();
			   (cli::safe_cast<System::ComponentModel::ISupportInitialize^>(this->pictureBoxImg))->EndInit();
			   (cli::safe_cast<System::ComponentModel::ISupportInitialize^>(this->brightnessTrackbar))->EndInit();
			   (cli::safe_cast<System::ComponentModel::ISupportInitialize^>(this->blurTrackbar))->EndInit();
			   this->ResumeLayout(false);
			   this->PerformLayout();

		   }
	#pragma endregion
		private: System::Void exitToolStripMenuItem_Click(System::Object^ sender, System::EventArgs^ e)
		{
			Application::Exit();
		}
		private: System::Void label1_Click(System::Object^ sender, System::EventArgs^ e) {
		}
		private: System::Void menuStrip1_ItemClicked(System::Object^ sender, System::Windows::Forms::ToolStripItemClickedEventArgs^ e) {
		}
		private: System::Void openToolStripMenuItem_Click(System::Object^ sender, System::EventArgs^ e)
		{
			if (dlgOpen->ShowDialog() == System::Windows::Forms::DialogResult::OK)
			{
				try
				{
					bmpFront = (Bitmap^)Image::FromFile(dlgOpen->FileName);

					SaveOriginalImage(bmpFront);

					pictureBoxImg->Image = bmpFront;

					ASMCheckBox->Enabled = true;
					brightnessTrackbar->Enabled = true;
					blurTrackbar->Enabled = true;
					negativeCheckbox->Enabled = true;

					Brightness_cppTotal = 0.0;
					Brightness_cppCount = 0.0;
					Brightness_asmCount = 0.0;
					Brightness_asmTotal = 0.0;
					Negative_asmCount = 0.0;
					Negative_asmTotal = 0.0;
					Negative_cppCount = 0.0;
					Negative_cppTotal = 0.0;
				}
				catch (...)
				{
					MessageBox::Show("File could not be opened!");
				}
			}
		}
		private: System::Void brightnessTrackbar_Scroll(System::Object^ sender, System::EventArgs^ e)
		{
			bmpData = bmpFront->LockBits(imgRect, ImageLockMode::WriteOnly, PixelFormat::Format24bppRgb);
			long startTime, finishTime;
			if (ASMCheckBox->Checked)
			{
				startTime = clock();
				ASMAdjustBrightness((unsigned char*)bmpData->Scan0.ToPointer(), bmpOriginal, brightnessTrackbar->Value, imgSizeInBytes);
				finishTime = clock();
				Brightness_asmTotal += finishTime - startTime;
				Brightness_asmCount++;
				averageTimeLabel->Text = "Average using ASM: " + Math::Round(Brightness_asmTotal / Brightness_asmCount, 2) + "ms";
			}
			else
			{
				startTime = clock();
				AdjustBrightness((unsigned char*)bmpData->Scan0.ToPointer(), brightnessTrackbar->Value);
				finishTime = clock();
				Brightness_cppTotal += finishTime - startTime;
				Brightness_cppCount++;
				averageTimeLabel->Text = "Average using C++: " + Math::Round(Brightness_cppTotal / Brightness_cppCount, 2) + "ms";
			}
			bmpFront->UnlockBits(bmpData);

			pictureBoxImg->Image = bmpFront;
		}

		private: System::Void Form1_FormClosing(System::Object^ sender, System::Windows::Forms::FormClosingEventArgs^ e)
		{
			ClearOriginalImage();
		}

		private: System::Void Form1_Load(System::Object^ sender, System::EventArgs^ e)
		{

		}

		private: System::Void negativeCheckbox_CheckedChanged(System::Object^ sender, System::EventArgs^ e) 
		{
			bmpData = bmpFront->LockBits(imgRect, ImageLockMode::WriteOnly, PixelFormat::Format24bppRgb);
			bool isChecked = negativeCheckbox->Checked;
			long startTime, finishTime;

			if(ASMCheckBox->Checked)
			{
				startTime = clock();
				ASMNegativeIMG((unsigned char*)bmpData->Scan0.ToPointer(), bmpOriginal, imgSizeInBytes, isChecked);
				finishTime = clock();
				Negative_asmTotal += finishTime - startTime;
				Negative_asmCount++;
				averageTimeLabel->Text = "Average using ASM: " + Math::Round(Negative_asmTotal / Negative_asmCount, 2) + "ms";
			}
			else
			{
				startTime = clock();
				CPPNegativeIMG((unsigned char*)bmpData->Scan0.ToPointer(), bmpOriginal, imgSizeInBytes, isChecked);
				finishTime = clock();
				Negative_cppTotal += finishTime - startTime;
				Negative_cppCount++;
				averageTimeLabel->Text = "Average using C++: " + Math::Round(Negative_cppTotal / Negative_cppCount, 2) + "ms";
			}
			bmpFront->UnlockBits(bmpData);

			pictureBoxImg->Image = bmpFront;

		}

		private: System::Void blurTrackbar_Scroll(System::Object^ sender, System::EventArgs^ e) 
		{
			bmpData = bmpFront->LockBits(imgRect, ImageLockMode::WriteOnly, PixelFormat::Format24bppRgb);
			long startTime, finishTime;
			int blurWidth = blurTrackbar->Value;
			
			if (ASMCheckBox->Checked)
			{
				hBlur = new unsigned char[imgSizeInBytes];
				startTime = clock();
				ASMBlurIMG(bmpFront->Height,bmpFront->Width,bmpOriginal,(unsigned char*)bmpData->Scan0.ToPointer(),hBlur,blurWidth);
				finishTime = clock();
				Blur_asmTotal += finishTime - startTime;
				Blur_asmCount++;
				averageTimeLabel->Text = "Average using ASM: " + Math::Round(Blur_asmTotal / Blur_asmCount, 2) + "ms";
			}
			else
			{
				startTime = clock();
				CPPBlurIMG((unsigned char*)bmpData->Scan0.ToPointer(), blurWidth);
				finishTime = clock();
				Blur_cppTotal += finishTime - startTime;
				Blur_cppCount++;
				averageTimeLabel->Text = "Average using C++: " + Math::Round(Blur_cppTotal / Blur_cppCount, 2) + "ms";
				
			}
			bmpFront->UnlockBits(bmpData);
			pictureBoxImg->Image = bmpFront;
			delete hBlur;
		}


	}; 
	
}