/*
  ==============================================================================

   This file is part of the JUCE library.
   Copyright (c) 2017 - ROLI Ltd.

   JUCE is an open source library subject to commercial or open-source
   licensing.

   By using JUCE, you agree to the terms of both the JUCE 5 End-User License
   Agreement and JUCE 5 Privacy Policy (both updated and effective as of the
   27th April 2017).

   End User License Agreement: www.juce.com/juce-5-licence
   Privacy Policy: www.juce.com/juce-5-privacy-policy

   Or: You may also use this code under the terms of the GPL v3 (see
   www.gnu.org/licenses).

   JUCE IS PROVIDED "AS IS" WITHOUT ANY WARRANTY, AND ALL WARRANTIES, WHETHER
   EXPRESSED OR IMPLIED, INCLUDING MERCHANTABILITY AND FITNESS FOR PURPOSE, ARE
   DISCLAIMED.

  ==============================================================================
*/

#pragma once

//#include <exception>
//#include <iostream>
//#include <ostream>
//#include <stdexcept>
#include <string>
//#include <windows.h>
//#include <bits/stdc++.h> 

//==============================================================================
// This is Color selector popup with 
class JucerColourPropertyComponent  : public PropertyComponent
{
public:
    JucerColourPropertyComponent (const String& name,
                                  const bool canReset)
        : PropertyComponent (name)
    {
        colourPropEditor.reset (new ColourPropEditorComponent (this, canReset));
        addAndMakeVisible (colourPropEditor.get());
    }

    virtual void setColour (Colour newColour) = 0;
    virtual Colour getColour() const = 0;
    virtual void resetToDefault() = 0;

    void refresh() override
    {
        ((ColourPropEditorComponent*) getChildComponent (0))->refresh();
    }

    class ColourEditorComponent    : public Component,
                                     public ChangeListener,
									public KeyListener
    {
    public:
        ColourEditorComponent (const bool canReset)
            : canResetToDefault (canReset)
        {
        }

        void paint (Graphics& g) override
        {
            g.fillAll (Colours::grey);

            g.fillCheckerBoard (getLocalBounds().reduced (2, 2).toFloat(),
                                10.0f, 10.0f,
                                Colour (0xffdddddd).overlaidWith (colour),
                                Colour (0xffffffff).overlaidWith (colour));

            g.setColour (Colours::white.overlaidWith (colour).contrasting());
            g.setFont (Font (getHeight() * 0.6f, Font::bold));
            g.drawFittedText (colour.toDisplayString (true),
                              2, 1, getWidth() - 4, getHeight() - 1,
                              Justification::centred, 1);
        }

        virtual void setColour (Colour newColour) = 0;
        virtual void resetToDefault() = 0;

		

		bool keyPressed(const KeyPress &key, Component *originatingComponent) override {

			
			if (key.getModifiers() == ModifierKeys::ctrlModifier) {
				if (key.getTextCharacter() == 'v' || key.getTextCharacter() == 'V') {
					PasteColor();
					return true;
				}
			}

			return false;

		}

		/* Sets color from clipboard string (assumed in hex color format) */
		virtual void PasteColor() = 0;



        virtual Colour getColour() const = 0;



        void refresh()
        {
            const Colour col (getColour());

            if (col != colour)
            {
                colour = col;
                repaint();
            }
        }

		
		

        void mouseDown (const MouseEvent&) override
        {
            CallOutBox::launchAsynchronously (new ColourSelectorComp (this, canResetToDefault),
                                              getScreenBounds(), nullptr);
        }

        void changeListenerCallback (ChangeBroadcaster* source) override
        {
            const ColourSelector* const cs = (const ColourSelector*) source;

            if (cs->getCurrentColour() != getColour())
                setColour (cs->getCurrentColour());

			
        }

        class ColourSelectorComp   : public Component
        {
        public:
            ColourSelectorComp (ColourEditorComponent* owner_,
                                const bool canReset)
                : owner (owner_),
                  defaultButton ("Reset to Default"),
				PasteColorButton("Paste")
            {
                addAndMakeVisible (selector);
                selector.setName ("Colour");
                selector.setCurrentColour (owner->getColour());
                selector.addChangeListener (owner);

				addKeyListener(owner);

                if (canReset)
                {
                    addAndMakeVisible (defaultButton);

                    defaultButton.onClick = [this]
                    {
                        owner->resetToDefault();
                        owner->refresh();
                        selector.setCurrentColour (owner->getColour());
                    };
                }

				addAndMakeVisible(PasteColorButton);

				// hardyVeles patch
				PasteColorButton.onClick = [this] {

					owner->PasteColor();
					owner->refresh();
					selector.setCurrentColour(owner->getColour());
				};
				
                setSize (300, 400);
            }

            void resized() override
            {
				selector.setBounds(0, 0, getWidth(), getHeight() - 30);
				PasteColorButton.changeWidthToFitText(22);
                if (defaultButton.isVisible())
                {
                    
                    defaultButton.changeWidthToFitText(22);
                    defaultButton.setTopLeftPosition (10, getHeight() - 26);
					PasteColorButton.setTopLeftPosition(defaultButton.getX() + defaultButton.getWidth() + 10, getHeight() - 26);
                }
                else
                {
					PasteColorButton.setTopLeftPosition(10, getHeight() - 26);
                    //selector.setBounds (getLocalBounds());
                }

				
				
				
            }

        private:
            class ColourSelectorWithSwatches    : public ColourSelector
            {
            public:
                ColourSelectorWithSwatches()
                {
                }

                int getNumSwatches() const override
                {
                    return getAppSettings().swatchColours.size();
                }

                Colour getSwatchColour (int index) const override
                {
                    return getAppSettings().swatchColours [index];
                }

                void setSwatchColour (int index, const Colour& newColour) override
                {
                    getAppSettings().swatchColours.set (index, newColour);
                }
            };

            ColourEditorComponent* owner;
            ColourSelectorWithSwatches selector;
            TextButton defaultButton;
			TextButton PasteColorButton;
        };

    private:
        Colour colour;
        bool canResetToDefault;
    };



    class ColourPropEditorComponent     : public ColourEditorComponent
    {
        JucerColourPropertyComponent* const owner;

    public:
        ColourPropEditorComponent (JucerColourPropertyComponent* const owner_,
                                   const bool canReset)
            : ColourEditorComponent (canReset),
              owner (owner_)
        {}

		// hardyVeles patch
		void PasteColor() override {
			
			juce::String color_hex = SystemClipboard::getTextFromClipboard();
			if (color_hex.startsWith("#")) color_hex = color_hex.substring(1);
			
			
			if (color_hex.length() == 6) {
				unsigned int r, g, b;
				sscanf(color_hex.getCharPointer(), "%02x%02x%02x", &r, &g, &b);

				Colour newColour = Colour(r, g, b);
				setColour(newColour);
			} else if (color_hex.length() == 8) {

				unsigned int a, r, g, b;
				sscanf(color_hex.getCharPointer(), "%02%02x%02x%02x", &a, &r, &g, &b);

				Colour newColour = Colour(juce::uint8(r), juce::uint8(g), juce::uint8(b), juce::uint8(a));
				setColour(newColour);
			}
			//owner->setColour(newColour);

		}

        void setColour (Colour newColour) override
        {
            owner->setColour (newColour);
        }

        Colour getColour() const override
        {
            return owner->getColour();
        }

		

        void resetToDefault() override
        {
            owner->resetToDefault();
        }
    };

    std::unique_ptr<ColourPropEditorComponent> colourPropEditor;
};
