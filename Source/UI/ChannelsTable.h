#include <VisualizerEditorHeaders.h>

#pragma once

namespace AONode
{
    class DeviceEditor;

    //==============================================================================
    class TableComponent : public Component,
                           public TableListBoxModel
    {
    public:
        TableComponent()
        {

            loadData();

            addAndMakeVisible(table);

            table.setColour(ListBox::outlineColourId, Colours::grey);
            table.setOutlineThickness(1);

            if (columnList != nullptr)
            {
                for (auto *columnXml : columnList->getChildIterator())
                {
                    table.getHeader().addColumn(columnXml->getStringAttribute("name"),
                                                columnXml->getIntAttribute("columnId"),
                                                columnXml->getIntAttribute("width"),
                                                50,
                                                400,
                                                TableHeaderComponent::defaultFlags);
                }
            }

            table.getHeader().setSortColumnId(1, true);

            table.setMultipleSelectionEnabled(true);

            resized();
        }

        int getNumRows() override
        {
            return numRows;
        }

        void paintRowBackground(Graphics &g, int rowNumber, int /*width*/, int /*height*/, bool rowIsSelected) override
        {
            auto alternateColour = getLookAndFeel().findColour(ListBox::backgroundColourId).interpolatedWith(getLookAndFeel().findColour(ListBox::textColourId), 0.03f);
            if (rowIsSelected)
                g.fillAll(Colours::lightblue);
            else if (rowNumber % 2)
                g.fillAll(alternateColour);
        }

        void paintCell(Graphics &g, int rowNumber, int columnId,
                       int width, int height, bool rowIsSelected) override
        {
            g.setColour(rowIsSelected ? Colours::darkblue : getLookAndFeel().findColour(ListBox::textColourId));
            g.setFont(font);

            if (auto *rowElement = dataList->getChildElement(rowNumber))
            {
                auto text = rowElement->getStringAttribute(getAttributeNameForColumnId(columnId));

                g.drawText(text, 2, 0, width - 4, height, Justification::centredLeft, true);
            }

            g.setColour(getLookAndFeel().findColour(ListBox::backgroundColourId));
            g.fillRect(width - 1, 0, 1, height);
        }

        void sortOrderChanged(int newSortColumnId, bool isForwards) override
        {
            if (newSortColumnId != 0)
            {
                DataSorter sorter(getAttributeNameForColumnId(newSortColumnId), isForwards);
                dataList->sortChildElements(sorter);

                table.updateContent();
            }
        }

        Component *refreshComponentForCell(int rowNumber, int columnId, bool /*isRowSelected*/,
                                           Component *existingComponentToUpdate) override
        {
            if (columnId == 3 || columnId == 4 || columnId == 5)
            {
                auto *textLabel = static_cast<EditableTextCustomComponent *>(existingComponentToUpdate);

                if (textLabel == nullptr)
                    textLabel = new EditableTextCustomComponent(*this);

                textLabel->setRowAndColumn(rowNumber, columnId);
                return textLabel;
            }

            jassert(existingComponentToUpdate == nullptr);
            return nullptr;
        }

        int getColumnAutoSizeWidth(int columnId) override
        {
            int widest = 32;

            for (auto i = getNumRows(); --i >= 0;)
            {
                if (auto *rowElement = dataList->getChildElement(i))
                {
                    auto text = rowElement->getStringAttribute(getAttributeNameForColumnId(columnId));

                    widest = jmax(widest, font.getStringWidth(text));
                }
            }

            return widest + 8;
        }

        int getSelection(const int rowNumber) const
        {
            return dataList->getChildElement(rowNumber)->getIntAttribute("Select");
        }

        void setSelection(const int rowNumber, const int newSelection)
        {
            dataList->getChildElement(rowNumber)->setAttribute("Select", newSelection);
        }

        String getText(const int columnNumber, const int rowNumber) const
        {
            return dataList->getChildElement(rowNumber)->getStringAttribute(getAttributeNameForColumnId(columnNumber));
        }

        void setText(const int columnNumber, const int rowNumber, const String &newText)
        {
            const auto &columnName = table.getHeader().getColumnName(columnNumber);
            dataList->getChildElement(rowNumber)->setAttribute(columnName, newText);
        }

        //==============================================================================
        void resized() override
        {
            table.setBoundsInset(BorderSize<int>(8));
        }

    private:
        TableListBox table{{}, this};
        Font font{14.0f};

        XmlElement *columnList = nullptr;
        XmlElement *dataList = nullptr;
        int numRows = 0;

        //==============================================================================
        class EditableTextCustomComponent : public Label
        {
        public:
            EditableTextCustomComponent(TableComponent &td)
                : owner(td)
            {
                setEditable(false, true, false);
            }

            void mouseDown(const MouseEvent &event) override
            {
                owner.table.selectRowsBasedOnModifierKeys(row, event.mods, false);

                Label::mouseDown(event);
            }

            void textWasEdited() override
            {
                owner.setText(columnId, row, getText());
            }

            void setRowAndColumn(const int newRow, const int newColumn)
            {
                row = newRow;
                columnId = newColumn;
                setText(owner.getText(columnId, row), dontSendNotification);
            }

        private:
            TableComponent &owner;
            int row, columnId;
            Colour textColour;
        };

        //==============================================================================
        class DataSorter
        {
        public:
            DataSorter(const String &attributeToSortBy, bool forwards)
                : attributeToSort(attributeToSortBy),
                  direction(forwards ? 1 : -1)
            {
            }

            int compareElements(XmlElement *first, XmlElement *second) const
            {
                auto result = first->getStringAttribute(attributeToSort)
                                  .compareNatural(second->getStringAttribute(attributeToSort));

                if (result == 0)
                    result = first->getStringAttribute("ID")
                                 .compareNatural(second->getStringAttribute("ID"));

                return direction * result;
            }

        private:
            String attributeToSort;
            int direction;
        };

        //==============================================================================
        void loadData()
        {
            // HEADERS
            columnList = new XmlElement("HEADERS");

            XmlElement *h1 = new XmlElement("1");
            h1->setAttribute("columnId", "1");
            h1->setAttribute("name", "ID");
            h1->setAttribute("width", 100);
            columnList->addChildElement(h1);

            XmlElement *h2 = new XmlElement("2");
            h2->setAttribute("columnId", "2");
            h2->setAttribute("name", "Name");
            h2->setAttribute("width", 200);
            columnList->addChildElement(h2);

            XmlElement *h3 = new XmlElement("3");
            h3->setAttribute("columnId", "3");
            h3->setAttribute("name", "Sampling Rate");
            h3->setAttribute("width", 100);
            columnList->addChildElement(h3);

            XmlElement *h4 = new XmlElement("4");
            h4->setAttribute("columnId", "4");
            h4->setAttribute("name", "Bit resolution");
            h4->setAttribute("width", 100);
            columnList->addChildElement(h4);

            XmlElement *h5 = new XmlElement("5");
            h5->setAttribute("columnId", "5");
            h5->setAttribute("name", "Gain");
            h5->setAttribute("width", 100);
            columnList->addChildElement(h5);

            // DATA
            dataList = new XmlElement("DATA");

            XmlElement *channel01 = new XmlElement("1000");
            channel01->setAttribute("ID", "1000");
            channel01->setAttribute("Name", "Channel Name");
            channel01->setAttribute("Sampling Rate", 44000);
            channel01->setAttribute("Bit resolution", 38.147);
            channel01->setAttribute("Gain", 20);
            dataList->addChildElement(channel01);

            XmlElement *channel02 = new XmlElement("1001");
            channel02->setAttribute("ID", "1001");
            channel02->setAttribute("Name", "Other Channel Name");
            channel02->setAttribute("Sampling Rate", 44000);
            channel02->setAttribute("Bit resolution", 38.147);
            channel02->setAttribute("Gain", 20);
            dataList->addChildElement(channel02);

            numRows = dataList->getNumChildElements();
        }

        String getAttributeNameForColumnId(const int columnId) const
        {
            for (auto *columnXml : columnList->getChildIterator())
            {
                if (columnXml->getIntAttribute("columnId") == columnId)
                    return columnXml->getStringAttribute("name");
            }

            return {};
        }

        JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(TableComponent)
    };

    //==============================================================================
    class ChannelsMainComponent : public Component
    {
    public:
        //==============================================================================
        ChannelsMainComponent()
        {
            addAndMakeVisible(table);

            setSize(1200, 600);
        }

        void paint(Graphics &g) override
        {
            g.fillAll(getLookAndFeel().findColour(ResizableWindow::backgroundColourId));
        }

        void resized() override
        {
            table.setBounds(getLocalBounds());
        }

    private:
        //==============================================================================
        TableComponent table;

        JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(ChannelsMainComponent)
    };
}