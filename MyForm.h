#pragma once

namespace TicTacToeCpp {

	using namespace System;
	using namespace System::ComponentModel;
	using namespace System::Collections;
	using namespace System::Windows::Forms;
	using namespace System::Data;
	using namespace System::Drawing;

	/// <summary>
	/// Summary for MyForm
	/// </summary>
	public ref class MyForm : public System::Windows::Forms::Form
	{
	private:
		PictureBox^ gameImage;
		Image^ wireImage;
		Image^ batteryImage;
		Image^ lightOffImage;
		Image^ lightOnImage;
		Image^ cornerWireImage;
		array<bool, 2>^ markedCells;

	public:
		MyForm(void)
		{
			InitializeComponent();
			wireImage = Image::FromFile(
				System::IO::Path::Combine(
					AppDomain::CurrentDomain->BaseDirectory,
					L"Images\\Wire.png"
				)
			);
			cornerWireImage = Image::FromFile(
				System::IO::Path::Combine(
					AppDomain::CurrentDomain->BaseDirectory,
					L"Images\\Corner_Wire.png"
				)
			);
			batteryImage = Image::FromFile(
				System::IO::Path::Combine(
					AppDomain::CurrentDomain->BaseDirectory,
					L"Images\\Battery.png"
				)
			);

			lightOffImage = Image::FromFile(
				System::IO::Path::Combine(
					AppDomain::CurrentDomain->BaseDirectory,
					L"Images\\Light_Off.png"
				)
			);

			lightOnImage = Image::FromFile(
				System::IO::Path::Combine(
					AppDomain::CurrentDomain->BaseDirectory,
					L"Images\\Light_On.png"
				)
			);
			
			markedCells = gcnew array<bool, 2>(7, 7);
			gameImage = gcnew PictureBox();
			gameImage->Image = Image::FromFile(
				System::IO::Path::Combine(
					AppDomain::CurrentDomain->BaseDirectory,
					L"Images\\7x7_Board_Updated.png"
				)
			);
			gameImage->SizeMode = PictureBoxSizeMode::Zoom;
			gameImage->Dock = DockStyle::Fill;
			gameImage->MouseClick +=
				gcnew MouseEventHandler(this, &MyForm::GameImage_Click);
			gameImage->Paint +=
				gcnew PaintEventHandler(this, &MyForm::GameImage_Paint);

			this->Controls->Add(gameImage);

		
		}

		void GameImage_Click(Object^ sender, MouseEventArgs^ e)
		{
			double scaleX = (double)gameImage->ClientSize.Width /
				gameImage->Image->Width;

			double scaleY = (double)gameImage->ClientSize.Height /
				gameImage->Image->Height;

			double scale = Math::Min(scaleX, scaleY);

			int displayedWidth = (int)(gameImage->Image->Width * scale);
			int displayedHeight = (int)(gameImage->Image->Height * scale);

			int offsetX = (gameImage->ClientSize.Width - displayedWidth) / 2;
			int offsetY = (gameImage->ClientSize.Height - displayedHeight) / 2;

			// Approximate board boundaries in the original image
			int boardLeft = offsetX + (int)(725 * scale);
			int boardRight = offsetX + (int)(1880 * scale);

			int boardTop = offsetY + (int)(154 * scale);
			int boardBottom = offsetY + (int)(1310 * scale);

			// Ignore clicks outside the board
			if (e->X < boardLeft || e->X >= boardRight ||
				e->Y < boardTop || e->Y >= boardBottom)
				return;

			int column = (e->X - boardLeft) * 7 /
				(boardRight - boardLeft);

			int row = (e->Y - boardTop) * 7 /
				(boardBottom - boardTop);

			// Prevent wires from being placed on the battery or lightbulb
			if (row == 3 && (column == 0 || column == 6))
				return;

			markedCells[row, column] = !markedCells[row, column];

			gameImage->Invalidate();
		}

		void GameImage_Paint(Object^ sender, PaintEventArgs^ e)
		{
			double scaleX = (double)gameImage->ClientSize.Width /
				gameImage->Image->Width;

			double scaleY = (double)gameImage->ClientSize.Height /
				gameImage->Image->Height;

			double scale = Math::Min(scaleX, scaleY);

			int displayedWidth = (int)(gameImage->Image->Width * scale);
			int displayedHeight = (int)(gameImage->Image->Height * scale);

			int offsetX = (gameImage->ClientSize.Width - displayedWidth) / 2;
			int offsetY = (gameImage->ClientSize.Height - displayedHeight) / 2;

			int boardLeft = offsetX + (int)(725 * scale);
			int boardRight = offsetX + (int)(1880 * scale);

			int boardTop = offsetY + (int)(154 * scale);
			int boardBottom = offsetY + (int)(1310 * scale);

			double cellWidth = (double)(boardRight - boardLeft) / 7.0;
			double cellHeight = (double)(boardBottom - boardTop) / 7.0;

			// Track whether the battery can reach the lightbulb
			bool circuitComplete = false;

			// Track visited squares
			array<bool, 2>^ visited = gcnew array<bool, 2>(7, 7);

			// Queue of squares to explore
			System::Collections::Generic::Queue<System::Drawing::Point>^ queue =
				gcnew System::Collections::Generic::Queue<System::Drawing::Point>();

			// Start at the battery (column 0, row 3)
			queue->Enqueue(System::Drawing::Point(0, 3));
			visited[3, 0] = true;
			
			// Perform a breadth-first search to see if the lightbulb can be reached
			while (queue->Count > 0)
			{
				System::Drawing::Point point = queue->Dequeue();
				int row = point.Y;
				int column = point.X;

				// Check if we've reached the lightbulb
				if (row == 3 && column == 6)
				{
					circuitComplete = true;
				}

				// Explore neighboring squares
				array<System::Drawing::Point>^ neighbors = gcnew array<System::Drawing::Point>
				{
					System::Drawing::Point(column - 1, row),
					System::Drawing::Point(column + 1, row),
					System::Drawing::Point(column, row - 1),
					System::Drawing::Point(column, row + 1)
				};

				for each (System::Drawing::Point neighbor in neighbors)
				{
					int nRow = neighbor.Y;
					int nCol = neighbor.X; 

					if (nRow >= 0 && nRow < 7 && nCol >= 0 && nCol < 7 &&
						(markedCells[nRow, nCol] || (nRow == 3 && nCol == 6)) &&
						!visited[nRow, nCol])
					{
						queue->Enqueue(neighbor);
						visited[nRow, nCol] = true;
					}
				}
			}

			// Battery: middle row (3), first column (0)
			int componentWidth = (int)(cellWidth * 0.65);
			int componentHeight = (int)(cellHeight * 0.65);

			int batteryX = (int)(boardLeft + (cellWidth - componentWidth) / 2);
			int batteryY = (int)(boardTop + 3 * cellHeight +
				(cellHeight - componentHeight) / 2);

			e->Graphics->DrawImage(
				batteryImage,
				batteryX,
				batteryY,
				componentWidth,
				componentHeight
			);

			// Lightbulb: middle row (3), last column (6)
			int lightX = (int)(boardLeft + 6 * cellWidth +
				(cellWidth - componentWidth) / 2);
			int lightY = batteryY;

			e->Graphics->DrawImage(
				circuitComplete ? lightOnImage : lightOffImage,
				lightX,
				lightY,
				componentWidth,
				componentHeight
			);

			int totalValue = 0;

			for (int row = 0; row < 7; row++)
			{
				for (int column = 0; column < 7; column++)
				{
					if (markedCells[row, column])
					{
						if (visited[row, column])
						{
							totalValue += Math::Abs(row - 3) + 1;
						}
						// Detect neighboring wires
						bool up = row > 0 && markedCells[row - 1, column];
						bool down = row < 6 && markedCells[row + 1, column];
						bool left = (column > 0 && markedCells[row, column - 1])
							|| (row == 3 && column == 1);
						bool right = (column < 6 && markedCells[row, column + 1])
							|| (row == 3 && column == 5);

						// Detect a corner with exactly two perpendicular connections
						bool isCorner =
							(up && left && !down && !right) ||
							(up && right && !down && !left) ||
							(down && left && !up && !right) ||
							(down && right && !up && !left);

						// Detect straight vertical wires
						bool isVertical = (up || down) && !left && !right;

						int markerWidth = (int)(cellWidth * 0.65);
						int markerHeight = (int)(cellHeight * 0.65);

						int x = boardLeft + column * cellWidth +
							(cellWidth - markerWidth) / 2;

						int y = boardTop + row * cellHeight +
							(cellHeight - markerHeight) / 2;

						if (isCorner)
						{
							Image^ rotatedCorner = safe_cast<Image^>(
								cornerWireImage->Clone()
								);

							if (up && right)
								rotatedCorner->RotateFlip(RotateFlipType::Rotate90FlipNone);
							else if (down && right)
								rotatedCorner->RotateFlip(RotateFlipType::Rotate180FlipNone);
							else if (down && left)
								rotatedCorner->RotateFlip(RotateFlipType::Rotate270FlipNone);

							e->Graphics->DrawImage(
								rotatedCorner,
								x,
								y,
								markerWidth,
								markerHeight
							);

							delete rotatedCorner;
						}
						else
						{
							if (isVertical)
							{
								Image^ rotatedWire = safe_cast<Image^>(
									wireImage->Clone()
									);

								rotatedWire->RotateFlip(
									RotateFlipType::Rotate90FlipNone
								);

								e->Graphics->DrawImage(
									rotatedWire,
									x,
									y,
									markerWidth,
									markerHeight
								);

								delete rotatedWire;
							}
							else
							{
								e->Graphics->DrawImage(
									wireImage,
									x,
									y,
									markerWidth,
									markerHeight
								);
							}
						}
					}
				}
			}
			//Calculating Score
			if (circuitComplete) {
				String^ resultText;
				if (totalValue <= 5)
				{
					resultText = L"Perfect Optimization!";
				}
				else if (totalValue <= 24)
				{
					resultText = L"Mostly Optimal!";
				}
				else
				{
					resultText = L"Not Optimal.";
				}
				//Score Text Placement
				RectangleF pillRect(
					offsetX + (float)(2045 * scale),
					offsetY + (float)(1205 * scale),
					(float)(315 * scale),
					(float) (140 * scale)
				);
			System::Drawing::Font^ resultFont = gcnew System::Drawing::Font(L"Consolas",
				Math::Max(8.0f, (float)(22 * scale)),
				FontStyle::Bold
			);

			StringFormat^ format = gcnew StringFormat();
			format->Alignment = StringAlignment::Center;
			format->LineAlignment = StringAlignment::Center;

			e->Graphics->DrawString(
				resultText,
				resultFont,
				Brushes::Black,
				pillRect,
				format
			);

			delete resultFont;
			delete format;
			}
		}	

	protected:
		/// <summary>
		/// Clean up any resources being used.
		/// </summary>
		~MyForm()
		{
			if (components)
			{
				delete components;
			}
		}

	private:
		/// <summary>
		/// Required designer variable.
		/// </summary>
		System::ComponentModel::Container ^components;

#pragma region Windows Form Designer generated code
		/// <summary>
		/// Required method for Designer support - do not modify
		/// the contents of this method with the code editor.
		/// </summary>
		void InitializeComponent(void)
		{
			this->components = gcnew System::ComponentModel::Container();
			this->Size = System::Drawing::Size(1280,720);
			this->Text = L"ABCircuit";
			this->Padding = System::Windows::Forms::Padding(0);
			this->AutoScaleMode = System::Windows::Forms::AutoScaleMode::Font;
		}
#pragma endregion
	};
}
