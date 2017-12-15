package com.jaxbulsara.ledgradapp;

import android.content.Context;
import android.support.test.InstrumentationRegistry;
import android.support.test.runner.AndroidJUnit4;

import org.junit.Test;
import org.junit.runner.RunWith;

import static org.junit.Assert.*;

/**
<<<<<<< HEAD
 * Instrumentation test, which will execute on an Android device.
=======
 * Instrumented test, which will execute on an Android device.
>>>>>>> develop
 *
 * @see <a href="http://d.android.com/tools/testing">Testing documentation</a>
 */
@RunWith(AndroidJUnit4.class)
public class ExampleInstrumentedTest {
<<<<<<< HEAD
    @Test
    public void useAppContext() throws Exception {
        // Context of the app under test.
        Context appContext = InstrumentationRegistry.getTargetContext();

        assertEquals("com.jaxbulsara.ledgradapp", appContext.getPackageName());
    }
=======
	@Test
	public void useAppContext() throws Exception {
		// Context of the app under test.
		Context appContext = InstrumentationRegistry.getTargetContext();

		assertEquals("com.jaxbulsara.ledgradapp", appContext.getPackageName());
	}
>>>>>>> develop
}
