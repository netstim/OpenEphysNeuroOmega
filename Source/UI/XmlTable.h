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
        }

        void init(XmlElement *xmlList)
        {
            dataList = xmlList;
            numRows = dataList->getNumChildElements();
            setUpHeaders();

            addAndMakeVisible(table);

            table.setColour(ListBox::outlineColourId, Colours::grey);
            table.setOutlineThickness(1);

            if (columnList != nullptr)
            {
                for (auto *columnXml : columnList->getChildIterator())
                {
                    table.getHeader().addColumn(columnXml->getStringAttribute("name"),
                                                columnXml->getIntAttribute("columnId"),
                                                getColumnAutoSizeWidth(columnXml->getIntAttribute("columnId")),
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
            String columnName = getAttributeNameForColumnId(columnId);

            if (columnName == "Sampling_Rate" || columnName == "Bit_Resolution" || columnName == "Gain" || columnName == "Channel_Name")
            {
                auto *textLabel = static_cast<EditableTextCustomComponent *>(existingComponentToUpdate);

                if (textLabel == nullptr)
                    textLabel = new EditableTextCustomComponent(*this);

                textLabel->setRowAndColumn(rowNumber, columnId);
                return textLabel;
            }
            else if (columnName == "Enabled")
            {
                auto *selectionBox = static_cast<SelectionColumnCustomComponent *>(existingComponentToUpdate);

                if (selectionBox == nullptr)
                    selectionBox = new SelectionColumnCustomComponent(*this);

                selectionBox->setRowAndColumn(rowNumber, columnId);
                return selectionBox;
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

            widest = jmax(widest, font.getStringWidth(getAttributeNameForColumnId(columnId)));

            return widest + 8;
        }

        bool getSelection(const int rowNumber) const
        {
            return dataList->getChildElement(rowNumber)->getBoolAttribute("Enabled");
        }

        void setSelection(const int rowNumber, const bool newSelection, juce::ToggleButton *toggleButton)
        {
            dataList->getChildElement(rowNumber)->setAttribute("Enabled", newSelection);
            if (atLeastOneStreamEnabled())
                xmlModifiedBroadcaster.sendActionMessage("Xml Modified");
            else
            {
                dataList->getChildElement(rowNumber)->setAttribute("Enabled", true);
                toggleButton->setToggleState(true, juce::dontSendNotification);
                AlertWindow::showMessageBox(AlertWindow::NoIcon, "Neuro Omega", "At least one must be enabled", "OK", nullptr);
            }
        }

        bool atLeastOneStreamEnabled()
        {
            for (auto i = getNumRows(); --i >= 0;)
            {
                if (auto *rowElement = dataList->getChildElement(i))
                {
                    if (rowElement->getBoolAttribute("Enabled"))
                        return true;
                }
            }
            return false;
        }

        String getText(const int columnNumber, const int rowNumber) const
        {
            return dataList->getChildElement(rowNumber)->getStringAttribute(getAttributeNameForColumnId(columnNumber));
        }

        void setText(const int columnNumber, const int rowNumber, const String &newText)
        {
            const auto &columnName = table.getHeader().getColumnName(columnNumber);
            dataList->getChildElement(rowNumber)->setAttribute(columnName, newText);
            xmlModifiedBroadcaster.sendActionMessage("Xml Modified");
        }

        //==============================================================================
        void resized() override
        {
            table.setBoundsInset(BorderSize<int>(8));
        }

        void updateContent()
        {
            if (dataList != nullptr)
                table.updateContent();
        }

        ActionBroadcaster xmlModifiedBroadcaster;

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

        class SelectionColumnCustomComponent : public Component
        {
        public:
            SelectionColumnCustomComponent(TableComponent &td)
                : owner(td)
            {
                toggleButton.setColour(ToggleButton::tickDisabledColourId, juce::Colour(220, 220, 220));
                addAndMakeVisible(toggleButton);

                toggleButton.onClick = [this]
                { owner.setSelection(row, (bool)toggleButton.getToggleState(), &toggleButton); };
            }

            void resized() override
            {
                toggleButton.setBoundsInset(juce::BorderSize<int>(2));
            }

            void setRowAndColumn(int newRow, int newColumn)
            {
                row = newRow;
                columnId = newColumn;
                toggleButton.setToggleState((bool)owner.getSelection(row), juce::dontSendNotification);
            }

        private:
            TableComponent &owner;
            juce::ToggleButton toggleButton;
            int row, columnId;
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
        void setUpHeaders()
        {
            columnList = new XmlElement("HEADERS");

            XmlElement *element;

            for (int i = 0; i < dataList->getChildElement(0)->getNumAttributes(); i++)
            {
                element = new XmlElement("COLUMN");
                element->setAttribute("columnId", i + 1);
                element->setAttribute("name", dataList->getChildElement(0)->getAttributeName(i));
                columnList->addChildElement(element);
            }
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
    class XmlTableMainComponent : public Component
    {
    public:
        //==============================================================================
        XmlTableMainComponent()
        {
        }

        void initFromXml(XmlElement *xmlList)
        {
            deleteAllChildren();
            table = new TableComponent();
            table->init(xmlList);
            addAndMakeVisible(table);
            setSize(1200, 600);
        }

        void paint(Graphics &g) override
        {
            g.fillAll(getLookAndFeel().findColour(ResizableWindow::backgroundColourId));
        }

        void resized() override
        {
            if (table != nullptr)
                table->setBounds(getLocalBounds());
        }

        void addXmlModifiedListener(ActionListener *l)
        {
            if (table != nullptr)
                table->xmlModifiedBroadcaster.addActionListener(l);
        }

        void updateContent()
        {
            if (table != nullptr)
                table->updateContent();
        }

        void setEnabled(bool shouldBeEnabled)
        {
            if (table != nullptr)
                table->setEnabled(shouldBeEnabled);
        }

    private:
        //==============================================================================
        TableComponent *table;
        JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(XmlTableMainComponent)
    };
}